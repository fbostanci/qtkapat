//https://github.com/mygit03/ShutDown/blob/master/mytrayicon.h
#ifndef MYTRAYICON_H
#define MYTRAYICON_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>

class MyTrayIcon : public QWidget
{
    Q_OBJECT
public:
    explicit MyTrayIcon(QStringList &strList, QWidget *parent = 0);
    ~MyTrayIcon();

    void createAct(QWidget *parent);
    void createMenu();

private:
    QSystemTrayIcon *trayIcon;
    QAction *minAct, *maxAct, *quitAct;
    QMenu *trayMenu;

    QWidget *doubleClickWidget;

private slots:
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
};

#endif // MYTRAYICON_H
