﻿//--------------------------------------------------------------------------+
//                                                                          *
//    "Qtkapat: GNU/Linux ve Windows için süre ayarlı sistem kapatıcı"      *
//              Copyright(C) 2020, FB <ironic{at}yaani.com>                 *
//                 https://gitlab.com/fbostanci/qtkapat                     *
//                            qtkapat v1.0                                  *
//                               GPLv3                                      *
//                                                                          *
//--------------------------------------------------------------------------+
//                                                                          *
//    This program is free software: you can redistribute it and/or modify  *
//    it under the terms of the GNU General Public License as published by  *
//    the Free Software Foundation, either version 3 of the License, or     *
//    (at your option) any later version.                                   *
//                                                                          *
//    This program is distributed in the hope that it will be useful,       *
//    but WITHOUT ANY WARRANTY; without even the implied warranty of        *
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
//    GNU General Public License for more details.                          *
//                                                                          *
//    You should have received a copy of the GNU General Public License     *
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.*
//                                                                          *
//--------------------------------------------------------------------------+

#include "qtkapat.h"
#include "ui_qtkapat.h"


#include <QTimer>
#include <QTime>
#include <QDate>
#include <QThread>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDebug>

#if defined(Q_OS_LINUX)
    constexpr int BU_BIR_LINUX = 1;
#elif defined(Q_OS_WIN)
    constexpr int BU_BIR_LINUX = 0;

    QString kapat_komutu = "shutdown /p /f";
    QString ybaslat_komutu = "shutdown /r /f";
    QString o_kapat_komutu = "shutdown /l";
    QString askiya_al_komutu = "rundll32.exe powrprof.dll,SetSuspendState 0,1,0";
#endif


Qtkapat::Qtkapat(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Qtkapat)
{
    ui->setupUi(this);
    this->setFixedWidth(609);
    this->setFixedHeight(375);
    this->setWindowTitle("QtKapat v1.0");

    dugmeAyarlari();
    linuxKomutlari();
    createActions();
    createTrayIcon();

    // bir görev düğmesi(radiobutton) seçili ise, diğerlerini etkisiz yap.
    connect(ui->radioButton_bz,SIGNAL(clicked()),this ,SLOT(gorevDugmeleri()));
    connect(ui->radioButton_bs,SIGNAL(clicked()),this ,SLOT(gorevDugmeleri()));
    connect(ui->radioButton_dk,SIGNAL(clicked()),this ,SLOT(gorevDugmeleri()));
    connect(ui->radioButton_sy,SIGNAL(clicked()),this ,SLOT(gorevDugmeleri()));

    // arayüz saat:
    bir_saniye = new QTimer(this);
    connect(bir_saniye, SIGNAL(timeout()), this, SLOT(zamaniGuncelle()));
    bir_saniye->start(1000);

    zamanlayici = new QTimer(this);
    zamanlayici->setInterval(1000);
    zamanlayici->setSingleShot(false);
    connect(zamanlayici, SIGNAL(timeout()), this, SLOT(slot_zamanlayici()));
}

Qtkapat::~Qtkapat()
{
    delete ui;
    delete bir_saniye;
    delete zamanlayici;
    delete trayIcon;
    delete trayIconMenu;
    delete gizle;
    delete goster;
    delete cikis;
}

void Qtkapat::gorevDugmeleri()
{
    // Belirtilen zaman düğmesi
    if (ui->radioButton_bz->isChecked()) {
        ui->dateTimeEdit_bz->setEnabled(true);
        ui->timeEdit_bs->setEnabled(false);
        ui->spinBox_dk->setEnabled(false);

      // Belirtilen saat düğmesi
    } else if (ui->radioButton_bs->isChecked()) {
        ui->dateTimeEdit_bz->setEnabled(false);
        ui->timeEdit_bs->setEnabled(true);
        ui->spinBox_dk->setEnabled(false);

      // Belirtilen dakika düğmesi
    } else if (ui->radioButton_dk->isChecked()) {
        ui->dateTimeEdit_bz->setEnabled(false);
        ui->timeEdit_bs->setEnabled(false);
        ui->spinBox_dk->setEnabled(true);

      // Şimdi düğmesi
    } else if (ui->radioButton_sy->isChecked()) {
        ui->dateTimeEdit_bz->setEnabled(false);
        ui->timeEdit_bs->setEnabled(false);
        ui->spinBox_dk->setEnabled(false);
    }
}

void Qtkapat::dugmeAyarlari()
{
    ui->label_us->setText("Qtkapat -> Görev ve işlem zamanını belirleyin.");
    //Tarih, saat ve dakika belirleme seçilene kadar etkin değil.
    ui->dateTimeEdit_bz->setEnabled(false);
    ui->timeEdit_bs->setEnabled(false);
    ui->spinBox_dk->setEnabled(false);
    //Belirtilen saat:
    ui->timeEdit_bs->setTime(QTime::currentTime().addSecs(60));
    ui->timeEdit_bs->setMinimumTime(QTime::currentTime().addSecs(60));
    // Belirtilen dakika
    ui->spinBox_dk->setValue(5);
    //Belirtilen zaman: tarih ve saat ayarı
    ui->dateTimeEdit_bz->setDisplayFormat("dd.MM.yyyy hh:mm");
    ui->dateTimeEdit_bz->setDateTime(QDateTime::currentDateTime().addSecs(60));
    ui->dateTimeEdit_bz->setMinimumDateTime(QDateTime::currentDateTime());
    ui->dateTimeEdit_bz->setMaximumDateTime(QDateTime::currentDateTime().addMonths(2));

    // İptal Et düğmesi
    ui->pushButton_ip->setEnabled(false);
    ui->pushButton_ip->setIcon(QIcon(":/images/cancel.png"));
    ui->pushButton_ip->setIconSize(QSize(20,20));

    // görevi başlat düğmesi
    ui->pushButton_gb->setIcon(QIcon(":/images/start.png"));
    ui->pushButton_gb->setIconSize(QSize(20,20));
    // hakkında düğmesi
    ui->pushButton_hk->setIcon(QIcon(":/images/info.png"));
    ui->pushButton_hk->setIconSize(QSize(20,20));
    ui->pushButton_hk->setStyleSheet("QPushButton {border-style: outset; border-width: 0px;}");

}

void Qtkapat::linuxKomutlari()
{
    if (BU_BIR_LINUX == 1) {
        bash = new QProcess(this);

        bash->start("bash", QStringList()<<"-c"<<"if [[ -n $KDE_SESSION_UID ]];then echo kde;fi");
        bash->waitForFinished();
        QString output = bash->readAllStandardOutput();
        output = output.trimmed();

        bash->start("bash", QStringList()<<"-c"<<"id -u -n");
        bash->waitForFinished();
        QString user = bash->readAllStandardOutput();
        user = user.trimmed();

        if (QString::compare(user,"root") == 0) {
            ui->radioButton_kt->setEnabled(false);
            ui->radioButton_yb->setEnabled(false);
            ui->radioButton_ok->setEnabled(false);
            ui->radioButton_as->setEnabled(false);
            ui->radioButton_bz->setEnabled(false);
            ui->radioButton_bs->setEnabled(false);
            ui->radioButton_dk->setEnabled(false);
            ui->radioButton_sy->setEnabled(false);
            ui->dateTimeEdit_bz->setEnabled(false);
            ui->timeEdit_bs->setEnabled(false);
            ui->pushButton_gb->setEnabled(false);
            ui->spinBox_uy->setEnabled(false);
            ui->spinBox_dk->setEnabled(false);
            ui->label_us->setText("<b>Qtkapat -> root haklarıyla kullanılamaz!!!</b>");
        }

        if (QString::compare(output,"kde") == 0) {
            kapat_komutu = "qdbus org.kde.ksmserver /KSMServer logout 0 2 2";
            ybaslat_komutu = "qdbus org.kde.ksmserver /KSMServer logout 0 1 2";
            o_kapat_komutu = "qdbus org.kde.ksmserver /KSMServer logout 0 3 3";
            askiya_al_komutu = ("qdbus org.kde.Solid.PowerManagement"
                                " /org/freedesktop/PowerManagement Suspend");
            qDebug("KDE");

        } else {  //systemd
            kapat_komutu = "systemctl poweroff";
            ybaslat_komutu = "systemctl restart";
            o_kapat_komutu = QStringLiteral("loginctl terminate-user %1").arg(user);
            askiya_al_komutu ="systemctl suspend";
            qDebug("systemd");
        }
    }
}

void Qtkapat::createActions()
{
    gizle = new QAction(QString("Gizle"), this);
    connect(gizle, SIGNAL(triggered()), this, SLOT(hide()));
    goster = new QAction(QString("Göster"), this);
    connect(goster, SIGNAL(triggered()), this, SLOT(show()));
    cikis = new QAction(QString("Çıkış"), this);
    connect(cikis, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void Qtkapat::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(goster);
    trayIconMenu->addAction(gizle);
    trayIconMenu->addAction(cikis);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    QIcon icon(":/images/shutdown.png");
    trayIcon->setIcon(icon);
    trayIcon->setToolTip("Qtkapat v1.0");
    QString ileti = "Sistem tepsisinde başlatıldı";
    trayIcon->show();
    trayIcon->showMessage("Qtkapat", ileti, QSystemTrayIcon::Information, 1000);
}

void Qtkapat::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible())
    {
        this->hide();
        event->ignore();
    }
}

void Qtkapat::zamaniGuncelle()
{
    QDateTime tarih = QDateTime::currentDateTime();
    QString tarihStr = tarih.toString("dd.MM.yyyy hh:mm:ss");
    // arayüz bugünün tarihi
    ui->label_st->setText(tarihStr);
}

void Qtkapat::islemZamani()
{
    // Belirtilen zaman düğmesi
    if (ui->radioButton_bz->isChecked()) {
        ui->dateTimeEdit_bz->setEnabled(true);
        ui->timeEdit_bs->setEnabled(false);
        ui->spinBox_dk->setEnabled(false);

        qint64 simdikiZaman = QDateTime::currentSecsSinceEpoch();
        QDateTime secilenZamanS= ui->dateTimeEdit_bz->dateTime();
        qint64 secilenZaman = secilenZamanS.toSecsSinceEpoch() -
                              (ui->dateTimeEdit_bz->time().second());
        hedef_sure = secilenZaman - simdikiZaman;

      // Belirtilen saat düğmesi
    } else if (ui->radioButton_bs->isChecked()) {
        ui->dateTimeEdit_bz->setEnabled(false);
        ui->timeEdit_bs->setEnabled(true);
        ui->spinBox_dk->setEnabled(false);

        QTime simdikiSaatS =QTime::currentTime();
        int simdikiSaat = QTime(0,0,0).secsTo(simdikiSaatS);
        int secilenSaat = (ui->timeEdit_bs->time().hour() *3600) +
                          (ui->timeEdit_bs->time().minute() *60);
        hedef_sure = secilenSaat - simdikiSaat;

      // Belirtilen dakika düğmesi
    } else if (ui->radioButton_dk->isChecked()) {
        ui->dateTimeEdit_bz->setEnabled(false);
        ui->timeEdit_bs->setEnabled(false);
        ui->spinBox_dk->setEnabled(true);

        hedef_sure = (ui->spinBox_dk->value() * 60);
    }

    if (hedef_sure <= 0) {
        QMessageBox::warning(this, "Qtkapat",
                             "Geçmiş süre!",
                             QMessageBox::Ok);
    } else {
        ui->pushButton_gb->setEnabled(false);
        ui->pushButton_ip->setEnabled(true);
        zamanlayici->start();
    }
}

//hakkında düğmesi
void Qtkapat::on_pushButton_hk_clicked()
{
    QMessageBox::about( this, "Qtkapat 1.0",
    "<h4>Süre ayarlı sistem kapatıcı</h4>"
    "Copyright (c) 2020, FB "
    "<a href=\"https://gitlab.com/fbostanci/qtkapat\">Uygulama sayfası</a>");
}

// Görev Başlat Düğmesi
void Qtkapat::on_pushButton_gb_clicked()
{
    if (ui->radioButton_kt->isChecked()) { // kapat düğmesi seçili ise
                if (ui->radioButton_sy->isChecked()) { // şimdi düğmesi
                    ui->label_us->setText("Sisteminiz kapatılacak.");
                    QProcess::execute(kapat_komutu);
                } else {
                    gerisayimStr2 = " kapatılacak";
                    islemZamani();
                }

    } else if (ui->radioButton_yb->isChecked()) { // yeniden başlat düğmesi
                if (ui->radioButton_sy->isChecked()){
                    ui->label_us->setText("Sisteminiz yeniden başlatılacak");
                    QProcess::execute(ybaslat_komutu);
                } else {
                    gerisayimStr2 = " yeniden başlatılacak";
                    islemZamani();
                }

    } else if (ui->radioButton_ok->isChecked()) { // oturumu kapat düğmesi
                if(ui->radioButton_sy->isChecked()){
                   ui->label_us->setText("Oturumunuz kapatılacak.");
                    QProcess::execute(o_kapat_komutu);
                } else {
                    gerisayimStr2 =" oturumunuzu kapatacak";
                    islemZamani();
                }

    } else if (ui->radioButton_as->isChecked()) { //askıya al düğmesi
                if(ui->radioButton_sy->isChecked()){
                   ui->label_us->setText("Sisteminiz askıya alınacak");
                    QProcess::execute(askiya_al_komutu);
                } else {
                    gerisayimStr2 = " askıya alınacak";
                    islemZamani();
                }
    } else {
        ui->label_us->setText("Herhangi bir görev seçmediniz.");
        QApplication::processEvents();
        QThread::msleep(1200);
        ui->label_us->setText("Qtkapat -> Görev ve işlem zamanını belirleyin.");
    }
}

// İptal düğmesi
void Qtkapat::on_pushButton_ip_clicked()
{
    zamanlayici->stop();
    ui->label_us->setText("Qtkapat: İşleminiz iptal edildi.");
    QMessageBox::information(this, "Qtkapat",
                             "İşleminiz iptal edildi.",
                             QMessageBox::Ok);

    ui->label_us->setText("Qtkapat -> Görev ve işlem zamanını belirleyin.");
    ui->pushButton_ip->setEnabled(false);
    ui->pushButton_gb->setEnabled(true);
}

void Qtkapat::slot_zamanlayici()
{
    int uyar_spinbox = ui->spinBox_uy->value();
    int uyar_sure = uyar_spinbox * 60;
    QString uyar_ileti;
    uyar_ileti = QStringLiteral("Seçtiğiniz görev %1 dk sonra gerçekleştirilecek").arg(uyar_spinbox);

    if (hedef_sure > 0) {
        int saat = hedef_sure / 3600;
        int dakika = hedef_sure % 3600 / 60;
        int saniye = hedef_sure % 60;

        QString gerisayimStr = QString("%1:%2:%3")
          .arg(saat, 2, 10, QChar('0'))
          .arg(dakika, 2, 10, QChar('0'))
          .arg(saniye, 2, 10, QChar('0'));

        ui->label_us->setText("Sisteminiz <b>" + gerisayimStr + "</b> sonra " + gerisayimStr2);
        trayIcon->setToolTip("Sisteminiz " + gerisayimStr + " sonra " + gerisayimStr2);
    }

    if ((hedef_sure - uyar_sure) == 0) {
        trayIcon->showMessage("Qtkapat", uyar_ileti, QSystemTrayIcon::Critical, 15000);
    }

    if (hedef_sure == 0) {
        ui->pushButton_ip->setEnabled(false);
        ui->pushButton_gb->setEnabled(true);
        zamanlayici->stop();

        if (ui->radioButton_kt->isChecked())
            QProcess::execute(kapat_komutu);
         else if (ui->radioButton_yb->isChecked())
            QProcess::execute(ybaslat_komutu);
         else if (ui->radioButton_ok->isChecked())
            QProcess::execute(o_kapat_komutu);
         else if (ui->radioButton_as->isChecked())
            QProcess::execute(askiya_al_komutu);   
    }
    hedef_sure--;
}
