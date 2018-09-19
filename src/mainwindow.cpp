#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QTextStream>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QMessageBox>

inline QString priceToString(int price){
    return QString::number(price/100.f,'f',2);
}

int MainWindow::getItemIndex(QString name)
{
    for (int i = 0; i<m_Items.size(); ++i)
        if (m_Items[i].name==name)
            return i;
    return -1;
}

bool MainWindow::parseReceipt(QByteArray jsonText)
{
    //document
        //receipt
            //items []
                //sum
                //name
                //quantity
            //dateTime
    QJsonDocument doc = QJsonDocument::fromJson(jsonText);

    // check validity of the document
    if(!doc.isNull())
    {
        if (doc.isObject()){
            QJsonValue document = doc.object().value("document");
            if (document.isObject()){
                QJsonValue vreceipt = document.toObject().value("receipt");
                if (vreceipt.isObject()){
                    QJsonObject receipt = vreceipt.toObject();
                    QJsonValue vitems = receipt.value("items");
                    if (vitems.isArray()){
                        QJsonArray items = vitems.toArray();
                        m_Items.clear();
                        for (int i =0; i<items.count(); ++i){
                            QJsonValue value = items.at(i);
                            if (value.isObject()){
                                QJsonObject item = value.toObject();
                                QJsonValue sum = item.value("sum");
                                QJsonValue name = item.value("name");
                                QJsonValue quantity = item.value("quantity");
                                if(sum.isDouble() && name.isString() && quantity.isDouble()){
                                    QString eName = name.toString().replace(";","",Qt::CaseInsensitive);
                                    int index = getItemIndex(eName);
                                    if (index==-1){
                                        sItem it;
                                        it.name = eName;
                                        it.price = sum.toInt();
                                        it.count = quantity.toDouble();
                                        m_Items.append(it);
                                    }
                                    else{
                                        m_Items[index].count+=quantity.toDouble();
                                        m_Items[index].price+=sum.toInt();
                                    }
                                }
                            }
                        }

                        QJsonValue vdateTime = receipt.value("dateTime");
                        if (vdateTime.isString()){
                            m_ReceiptDateTime = QDateTime::fromString(vdateTime.toString(),"yyyy-MM-ddTHH:mm:ss");
                            m_DateTimeAvaialble = true;
                        }
                        else{
                            m_DateTimeAvaialble = false;
                        }
                        return true;
                    }
                }
            }
        }
    }
    else{
        qWarning() << "not json";
    }
    return false;
}

void MainWindow::reinitItems()
{
    for (int i = 0; i<m_Items.count(); ++i){
        auto itemIt = m_SavedItems.find(m_Items[i].name);
        m_Items[i].warrantyPeriod = 0;
        m_Items[i].warrantyType = NONE;
        if (itemIt==m_SavedItems.end()){
            m_Items[i].category = "";
            m_Items[i].countFactor = 1;
            m_Items[i].countType = "";
            m_Items[i].newname = "";
        }
        else{
            m_Items[i].category = itemIt.value().category;
            m_Items[i].countFactor = itemIt.value().countFactor;
            m_Items[i].countType = itemIt.value().countType;
            m_Items[i].newname = itemIt.value().newName;
        }
    }
}

void MainWindow::loadItems()
{
    QFile textFile("items.csv");
    if (textFile.open(QIODevice::ReadOnly)){
        m_SavedItems.clear();
        QTextStream textStream(&textFile);
        while (true)
        {
            QString line = textStream.readLine();
            if (line.isNull())
                break;
            else{
                QStringList row = line.split(";",QString::KeepEmptyParts);
                if (row.count()!=5)
                    break;
                else{
                    sSavedItem item;
                    item.name = row[0];
                    item.newName = row[1];
                    item.category = row[2];
                    item.countFactor = row[3].toFloat();
                    item.countType = row[4];
                    m_SavedItems.insert(item.name,item);
                }
            }
        }
    }
}

void MainWindow::saveItems()
{
    loadItems();//load actual items, update, and save.
    for (int i = 0; i<m_Items.count(); ++i){
        auto itemIt = m_SavedItems.find(m_Items[i].name);
        if (itemIt==m_SavedItems.end()){
            sSavedItem item;
            item.category = m_Items[i].category;
            item.countFactor = m_Items[i].countFactor;
            item.countType = m_Items[i].countType;
            item.newName = m_Items[i].newname;
            item.name = m_Items[i].name;
            m_SavedItems.insert(item.name,item);
        }
        else{
            itemIt.value().category = m_Items[i].category;
            itemIt.value().countFactor = m_Items[i].countFactor;
            itemIt.value().countType = m_Items[i].countType;
            itemIt.value().newName = m_Items[i].newname;
        }
    }

    QFile textFile("items.csv");
    if (textFile.open(QIODevice::WriteOnly)){
        auto it = m_SavedItems.begin();
        while (it!=m_SavedItems.end()){
            QString line = it.value().name + ";"+
                            it.value().newName + ";"+
                            it.value().category + ";"+
                            QString::number(it.value().countFactor) + ";"+
                            it.value().countType+"\n";
            textFile.write(line.toUtf8());
            ++it;
        }
    };
}



QString roundPrice(int price){
    return QString::number(price / 100 + (price % 100> 0? 1: 0));
}

QString roundCount(float count){
    int c = count* 100;
    QString v = QString::number(c/100);
    c = c % 100;
    if (c!=0)
        v+="."+QString::number(c);
    return v;
}

QStringList MainWindow::generateCSV()
{
    QVector<sItem> items;
    for (int i = 0; i<m_Items.count(); ++i){
        int index = -1;
        for (int j = 0; j<items.count(); ++j){
            if (items[j].newname==m_Items[i].newname &&
                items[j].category==m_Items[i].category &&
                abs(items[j].countFactor-m_Items[i].countFactor)<0.01 &&
                items[j].countType==m_Items[i].countType &&
                items[j].warrantyType==m_Items[i].warrantyType &&
                items[j].warrantyPeriod==m_Items[i].warrantyPeriod
                    ){
                index = j;
                break;
            }
        }

        if (index==-1){
            sItem item;
            item.newname = m_Items[i].newname;
            item.category = m_Items[i].category;
            item.price = m_Items[i].price;
            item.countType = m_Items[i].countType;
            item.countFactor = m_Items[i].countFactor;
            item.count = m_Items[i].count;
            item.warrantyType = m_Items[i].warrantyType;
            item.warrantyPeriod = m_Items[i].warrantyPeriod;
            items.append(item);
        }
        else{
            items[index].price+=m_Items[i].price;
            items[index].count+=m_Items[i].count;
        }
    }

    QMap<QString, sCSVItem> categories;
    for (int i = 0; i<items.count(); ++i){
        QString itemText = roundCount(items[i].count*items[i].countFactor)+items[i].countType+ " ";
        itemText += items[i].newname;
        if (items[i].warrantyPeriod>0 && items[i].warrantyType!=NONE){
            QDate receiptDate = ui->dateTimeEdit->date();
            QDate finalDate;
            QString warrantyText = "Гарантия "+QString::number(items[i].warrantyPeriod);
            switch (items[i].warrantyType) {
            case NONE:
                break;
            case DAYS:
                finalDate = receiptDate.addDays(items[i].warrantyPeriod);
                warrantyText += " дней";
                break;
            case MONTHS:
                finalDate = receiptDate.addMonths(items[i].warrantyPeriod);
                warrantyText += " месяцев";
                break;
            case YEARS:
                finalDate = receiptDate.addYears(items[i].warrantyPeriod);
                warrantyText += " лет";
                break;
            }

            warrantyText += ", до "+finalDate.toString("dd.MM.yyyy");
            warrantyText = "["+warrantyText+"]";
            itemText+=warrantyText;
        }
        if (items[i].count>1)
            itemText+=" x"+roundCount(items[i].count);

        if (categories.contains(items[i].category)){
            sCSVItem& cat = categories[items[i].category];
            cat.text += ", ";
            cat.text += itemText;
            cat.price+=items[i].price;
        }
        else{
            sCSVItem cat;
            cat.text = itemText;
            cat.price = items[i].price;
            categories.insert(items[i].category,cat);
        }
    }

    QStringList csv;
    QMap<QString, sCSVItem>::const_iterator it;
    for (it = categories.constBegin(); it != categories.constEnd(); ++it){
        csv.append(roundPrice(it.value().price)+";"+it.value().text+";"+it.key());
    }
    return csv;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ScanDialog(this),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_FakeDeviceID = m_Settings.value("User/DeviceID",QUuid::createUuid().toString()).toString();
    ui->lineEditUser->setText(m_Settings.value("User/Phone").toString());
    ui->lineEditPassword->setText(m_Settings.value("User/Password").toString());

    connect(&m_NAM, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(replyFinished(QNetworkReply*)));

    loadItems();

    QFile textFile("categories.txt");
    if (textFile.open(QIODevice::ReadOnly)){
        QTextStream textStream(&textFile);
        while (true)
        {
            QString line = textStream.readLine();
            if (line.isNull())
                break;
            else
                m_Categories.append(line);
        }
    }
    m_Categories.sort();
    m_Categories.insert(0,"");
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString encod2eBase64( QString data )
{
 QByteArray text;
 text.append(data);
 return text.toBase64();
}

void MainWindow::sendRequest()
{

    QUrl url("https://proverkacheka.nalog.ru:9999/v1/inns/*/kkts/*/fss/"+m_ScanDialog.getFN()+"/tickets/"+m_ScanDialog.getFD());
    QUrlQuery query;

    query.addQueryItem("fiscalSign", m_ScanDialog.getFPD());
    query.addQueryItem("sendToEmail", "no");

    url.setQuery(query.query());

    QNetworkRequest request(url);

    request.setRawHeader("Authorization", ("Basic "+encod2eBase64(ui->lineEditUser->text()+":"+ui->lineEditPassword->text())).toUtf8());
    request.setRawHeader("Device-Id", m_FakeDeviceID.toUtf8());
    request.setRawHeader("Device-OS", "Adnroid 5.1");
    request.setRawHeader("Version", "2");
    request.setRawHeader("ClientVersion", "1.4.4.1");

    m_NAM.get(request);
}

void MainWindow::on_pushButton_clicked()
{
    if (m_ScanDialog.exec()){
        m_Settings.setValue("User/Phone", ui->lineEditUser->text());
        m_Settings.setValue("User/Password", ui->lineEditPassword->text());
        m_Settings.setValue("User/DeviceID", m_FakeDeviceID);
        m_Settings.sync();

        sendRequest();
    }
}

void MainWindow::replyFinished(QNetworkReply *reply)
{
    QByteArray response = reply->readAll();
    qDebug() << "response received";
    qDebug().noquote() << QString::fromUtf8(response);

    if (parseReceipt(response)){
        if (m_DateTimeAvaialble)
            ui->dateTimeEdit->setDateTime(m_ReceiptDateTime);
        else
            ui->dateTimeEdit->setDateTime(QDateTime());
        reinitItems();
        reinitTable();
    }
    else{
        if (QMessageBox::information(0,tr("Ошибка запроса"),tr("Запрос не прошел. Повторить?"),QMessageBox::Yes,QMessageBox::Cancel)==QMessageBox::Yes)
            sendRequest();
    }
}

void MainWindow::reinitTable()
{
    QStringList lst;
    auto it = m_SavedItems.begin();
    while (it!=m_SavedItems.end()){
        lst.append(it.value().newName);
        ++it;
    }
    lst.removeDuplicates();
    lst.sort();

    m_NewNameAutoComplete.setModel(new QStringListModel(lst));
    m_NewNameAutoComplete.setCaseSensitivity(Qt::CaseInsensitive);
    m_NewNameAutoComplete.setCompletionMode(QCompleter::UnfilteredPopupCompletion);

    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);

    ui->tableWidget->setColumnCount(10);
    ui->tableWidget->setShowGrid(true);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Название") << tr("Короткое название") << tr("Количество в штуке") << tr("Количество штук") << tr("Единицы") << tr("Цена за все") << tr("Категория") << tr("Общее количество") << tr("Период гарантии") << tr("Тип гарантии"));
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    for (int i = 0; i<m_Items.count(); ++i){
        ui->tableWidget->insertRow(i);
        ui->tableWidget->setItem(i,0, new QTableWidgetItem(m_Items[i].name));
        {
            QLineEdit* lineEdit = new QLineEdit();
            ui->tableWidget->setCellWidget(i,1,lineEdit);
            lineEdit->setText(m_Items[i].newname);
            lineEdit->setCompleter(&m_NewNameAutoComplete);
            lineEdit->setProperty("index",i);
            connect(lineEdit, SIGNAL(textChanged(QString)),this, SLOT(onNewNameValueChanged(QString)));
        }

        {
            QDoubleSpinBox* spinBox = new QDoubleSpinBox();
            ui->tableWidget->setCellWidget(i,2,spinBox);
            spinBox->setValue(m_Items[i].countFactor);
            spinBox->setProperty("index",i);
            connect(spinBox,SIGNAL(valueChanged(double)),this,SLOT(onCountFactorValueChanged(double)));
        }
        ui->tableWidget->setItem(i,3, new QTableWidgetItem(QString::number(m_Items[i].count)));

        {
            QComboBox* combo = new QComboBox();
            ui->tableWidget->setCellWidget(i,4,combo);
            combo->addItems(QStringList() << "" << "ШТ" << "Л" << "КГ" << "УП");
            combo->setCurrentText(m_Items[i].countType);
            combo->setProperty("index",i);
            connect(combo, SIGNAL(currentTextChanged(QString)),this, SLOT(onCountTypeValueChanged(QString)));
        }

        ui->tableWidget->setItem(i,5, new QTableWidgetItem(priceToString(m_Items[i].price)));

        {
            QComboBox* combo = new QComboBox();
            ui->tableWidget->setCellWidget(i,6,combo);
            combo->addItems(m_Categories);
            combo->setCurrentText(m_Items[i].category);
            combo->setProperty("index",i);
            combo->setEditable(true);
            connect(combo, SIGNAL(currentTextChanged(QString)),this, SLOT(onCategoryValueChanged(QString)));
        }

        ui->tableWidget->setItem(i,7, new QTableWidgetItem(m_Items[i].getTotalCount()));

        {
            QSpinBox* spinBox = new QSpinBox();
            ui->tableWidget->setCellWidget(i,8,spinBox);
            spinBox->setValue(m_Items[i].warrantyPeriod);
            spinBox->setProperty("index",i);
            spinBox->setMaximum(9999);
            connect(spinBox,SIGNAL(valueChanged(int)),this,SLOT(onWarrantyPeriodValueChanged(int)));
        }

        {
            QComboBox* combo = new QComboBox();
            ui->tableWidget->setCellWidget(i,9,combo);
            combo->addItems(QStringList() << "" << tr("ДНИ") << tr("МЕСЯЦЫ") << tr("ГОДА"));
            combo->setCurrentIndex((int)m_Items[i].warrantyType);
            combo->setProperty("index",i);
            connect(combo, SIGNAL(currentIndexChanged(int)),this, SLOT(onWarrantyTypeValueChanged(int)));
        }
    }
}

void MainWindow::onCountFactorValueChanged(double value)
{
    int index = sender()->property("index").toInt();
    m_Items[index].countFactor = value;
    ui->tableWidget->item(index,7)->setText(m_Items[index].getTotalCount());
}

void MainWindow::onCountTypeValueChanged(QString value)
{
    int index = sender()->property("index").toInt();
    m_Items[index].countType = value;
    ui->tableWidget->item(index,7)->setText(m_Items[index].getTotalCount());
}

void MainWindow::onCategoryValueChanged(QString value)
{
    int index = sender()->property("index").toInt();
    m_Items[index].category = value;
}

void MainWindow::onNewNameValueChanged(QString value)
{
    int index = sender()->property("index").toInt();
    m_Items[index].newname = value;
}

void MainWindow::onWarrantyPeriodValueChanged(int value)
{
    int index = sender()->property("index").toInt();
    m_Items[index].warrantyPeriod = value;
}

void MainWindow::onWarrantyTypeValueChanged(int value)
{
    int index = sender()->property("index").toInt();
    m_Items[index].warrantyType = (eWarrantyType)value;
}

void MainWindow::on_pushButton_2_clicked()
{
    for (int i = 0; i<m_Items.count(); ++i){
        m_Items[i].newname = m_Items[i].newname.replace(";","");
        if (m_Items[i].category.isEmpty()){
            QMessageBox::warning(0,tr("Data not ready"),tr("Category not set for item %1").arg(m_Items[i].name));
            return;
        }
        if (m_Items[i].newname.isEmpty()){
            QMessageBox::warning(0,tr("Data not ready"),tr("New name not set for item %1").arg(m_Items[i].name));
            return;
        }
        if (m_Items[i].countType.isEmpty()){
            QMessageBox::warning(0,tr("Data not ready"),tr("Counter type not set for item %1").arg(m_Items[i].name));
            return;
        }
    }

    saveItems();
    m_CopyTextDialog.exec(generateCSV());
}
