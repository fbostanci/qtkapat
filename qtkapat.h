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
    void slot_zamanlayici();
    void ZamaniGuncelle();
    void createActions();

private:
    Ui::Qtkapat *ui;
    QTimer *zamanlayici;
    QTimer *bir_saniye;

    void createTrayIcon();
    void LinuxKomutlari();
    void IslemZamani();

    QAction *gizle;
    QAction *goster;
    QAction *cikis;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    int hedef_sure;
    QString gerisayimStr, gerisayimStr2;
};
#endif // QTKAPAT_H
