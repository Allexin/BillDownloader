#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "dialogscanreceipt.h"
#include "dialogcopytext.h"
#include <QSettings>
#include "cnetclient.h"
#include <QNetworkAccessManager>
#include <QCompleter>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    enum eWarrantyType{
        NONE = 0,
        DAYS,
        MONTHS,
        YEARS
    };

    struct sItem{
        QString name;
        QString newname;
        float count;
        float countFactor;
        QString countType;
        QString category;
        eWarrantyType warrantyType;
        int warrantyPeriod;
        int price;
        QString getTotalCount(){
            return QString::number(count*countFactor)+countType;
        }
    };

    struct sSavedItem{
        QString name;
        QString newName;
        QString category;
        float countFactor;
        QString countType;
    };

    struct sCSVItem{
        QString text;
        int price;
    };

    QCompleter          m_NewNameAutoComplete;
    QNetworkAccessManager m_NAM;
    QSettings           m_Settings;
    DialogScanReceipt      m_ScanDialog;
    DialogCopyText      m_CopyTextDialog;
    QVector<sItem>      m_Items;
    bool                m_DateTimeAvaialble;
    QDateTime           m_ReceiptDateTime;
    QStringList         m_Categories;
    QMap<QString,sSavedItem> m_SavedItems;
    QString             m_FakeDeviceID;
    int getItemIndex(QString name);
private:
    bool parseReceipt(QByteArray jsonText);
    void reinitItems();
    void reinitTable();

    void loadItems();
    void saveItems();
    void sendRequest();
    QStringList generateCSV();
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
private slots:
    void replyFinished(QNetworkReply* reply);
    void onCountFactorValueChanged(double value);
    void onCountTypeValueChanged(QString value);
    void onCategoryValueChanged(QString value);
    void onNewNameValueChanged(QString value);
    void onWarrantyPeriodValueChanged(int value);
    void onWarrantyTypeValueChanged(int value);
    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
