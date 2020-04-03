#ifndef QTKAPAT_H
#define QTKAPAT_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Qtkapat; }
QT_END_NAMESPACE

class Qtkapat : public QMainWindow
{
    Q_OBJECT

public:
    Qtkapat(QWidget *parent = nullptr);
    ~Qtkapat();

protected:
    void closeEvent(QCloseEvent *olay) override;

private slots:
    void on_pushButton_gb_clicked();
    void on_pushButton_ip_clicked();
    void slot_zamanlayici();
    void ZamaniGuncelle();
    void IslemZamani();


private:
    Ui::Qtkapat *ui;
    QTimer *zamanlayici;
    QTimer *gerisayim;
    QTimer *bir_saniye;
    int hedef_sure;
    QString gerisayimStr, gerisayimStr2;

};
#endif // QTKAPAT_H
