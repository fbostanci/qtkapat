#ifndef QTKAPAT_H
#define QTKAPAT_H

#include <QMainWindow>
#include <QWidget>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>

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
    void on_pushButton_hk_clicked();
    void gorevDugmeleri();
    void slot_zamanlayici();
    void zamaniGuncelle();
    void createActions();

private:
    Ui::Qtkapat *ui;
    QTimer *zamanlayici;
    QTimer *bir_saniye;

    void dugmeAyarlari();
    void createTrayIcon();
    void linuxKomutlari();
    void islemZamani();

    QAction *gizle;
    QAction *goster;
    QAction *cikis;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    int hedef_sure;
    QString gerisayimStr, gerisayimStr2;
#if defined(Q_OS_LINUX)
    QString kapat_komutu, ybaslat_komutu;
    QString o_kapat_komutu, askiya_al_komutu;
#endif
};
#endif // QTKAPAT_H
