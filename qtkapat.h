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
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_pushButton_gb_clicked();
    void on_pushButton_ip_clicked();
    void slot_zamanlayici();
    void ZamaniGuncelle();
    void LinuxKomutlari();
    void IslemZamani();


private:
    Ui::Qtkapat *ui;
    QTimer *zamanlayici;
    QTimer *bir_saniye;
    int hedef_sure;
    QString gerisayimStr, gerisayimStr2;
    QString kapat_komutu, ybaslat_komutu;
    QString o_kapat_komutu, askiya_al_komutu;

};
#endif // QTKAPAT_H
