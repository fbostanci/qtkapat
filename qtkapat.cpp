//--------------------------------------------------------------------------+
//                                                                          *
//    "Qtkapat: GNU/Linux ve Windows için süre ayarlı sistem kapatıcı"      *
//              Copyright(C) 2020, FB <ironic{at}yaani.com>                 *
//                 https://gitlab.com/fbostanci/qtkapat                     *
//                          qtkapat v0.0.1                                  *
//                              GPL v3                                      *
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


#include <QProcess>
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

    QString kapat_komutu = "shutdown -p";
    QString ybaslat_komutu = "shutdown -r -t 0";
    QString o_kapat_komutu = "shutdown -l";
    QString askiya_al_komutu = "rundll32.exe powrprof.dll,SetSuspendState 0,1,0";
#endif



Qtkapat::Qtkapat(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Qtkapat)
{
    ui->setupUi(this);
    this->setFixedWidth(609);
    this->setFixedHeight(375);
    this->setWindowTitle("QtKapat v0.0.1");

    LinuxKomutlari();
    createActions();
    createTrayIcon();

    zamanlayici = new QTimer(this);
    zamanlayici->setInterval(1000);
    zamanlayici->setSingleShot(false);
    connect(zamanlayici, SIGNAL(timeout()), this, SLOT(slot_zamanlayici()));


    // İptal Et düğmesi
    ui->pushButton_ip->setEnabled(false);
    ui->label_us->setText("Qtkapat -> Görev ve işlem zamanını belirleyin.");
    //Belirtilen saat:
    ui->timeEdit_bs->setTime(QTime::currentTime().addSecs(60));
    //Belirtilen zaman tarih:
    ui->dateTimeEdit_bz->setDisplayFormat("dd.MM.yyyy hh:mm");
    ui->dateTimeEdit_bz->setMinimumDateTime(QDateTime::currentDateTime());
    ui->dateTimeEdit_bz->setDateTime(QDateTime::currentDateTime().addSecs(60));

    // arayüz saat timer nesnesi
    bir_saniye = new QTimer(this);
    connect(bir_saniye, SIGNAL(timeout()), this, SLOT(ZamaniGuncelle()));
    bir_saniye->start(1000);
}

Qtkapat::~Qtkapat()
{
    delete ui;
}

void Qtkapat::LinuxKomutlari() {
    if (BU_BIR_LINUX == 1) {
        QProcess bash,bash2, bash3;
        bash.start("bash", QStringList()<<"-c"<<"if [[ -n $KDE_SESSION_UID ]];then echo kde;fi");
        bash.waitForFinished();
        QString output = bash.readAllStandardOutput();
        output = output.trimmed();

        bash2.start("bash", QStringList()<<"-c"<<"id -u -n");
        bash2.waitForFinished();
        QString user = bash2.readAllStandardOutput();
        user = user.trimmed();

        bash3.start("bash", QStringList()<<"-c"<<"if [[ -n $(pidof xfce4-session) ]];then echo xfce;fi");
        bash3.waitForFinished();
        QString output2 = bash3.readAllStandardOutput();
        output2 = output2.trimmed();

        if (user == "root") {
            ui->radioButton_kt->setEnabled(false);
            ui->radioButton_yb->setEnabled(false);
            ui->radioButton_ok->setEnabled(false);
            ui->radioButton_as->setEnabled(false);
            ui->label_us->setText("Qtkapat -> root haklarıyla kullanılamaz!!!");
        }

        if (output == "kde") {
            kapat_komutu = "qdbus org.kde.ksmserver /KSMServer logout 0 2 2";
            ybaslat_komutu = "qdbus org.kde.ksmserver /KSMServer logout 0 1 2";
            o_kapat_komutu = "qdbus org.kde.ksmserver /KSMServer logout 0 3 3";
            askiya_al_komutu = ("qdbus org.kde.Solid.PowerManagement" \
                                " /org/freedesktop/PowerManagement Suspend");
            qDebug("KDE");

        } else if (output2 == "xfce") {
            kapat_komutu = "xfce4-session-logout --halt";
            ybaslat_komutu = "xfce4-session-logout --reboot";
            o_kapat_komutu = "xfce4-session-logout --logout";
            askiya_al_komutu ="xfce4-session-logout --suspend";
            qDebug("xfce");

        } else {  //systemd
            kapat_komutu = "systemctl poweroff";
            ybaslat_komutu = "systemctl restart";
            o_kapat_komutu = QStringLiteral("loginctl terminate-user %1").arg(user);
            askiya_al_komutu ="systemctl suspend";
            qDebug("systemd");
        }
    }
}


void Qtkapat::createActions(){
    gizle = new QAction(QString("Gizle"), this);
    connect(gizle, SIGNAL(triggered()), this, SLOT(hide()));
    goster = new QAction(QString("Göster"), this);
    connect(goster, SIGNAL(triggered()), this, SLOT(show()));
    cikis = new QAction(QString("Çıkış"), this);
    connect(cikis, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void Qtkapat::createTrayIcon(){
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(goster);
    trayIconMenu->addAction(gizle);
    trayIconMenu->addAction(cikis);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    QIcon icon(":/images/shutdown.png");
    trayIcon->setIcon(icon);
    trayIcon->setToolTip("Qtkapat v0.0.1");
    QString ileti = "Sistem tepsisinde başlatıldı";
    trayIcon->show();
    trayIcon->showMessage("Qtkapat", ileti, QSystemTrayIcon::Information, 1000);
}

void Qtkapat::closeEvent(QCloseEvent *event)
{
    QMessageBox msgBox;

    msgBox.setWindowTitle("Qtkapat");
    msgBox.setWindowIcon(QIcon(":/images/shutdown.png"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("Simge durumuna küçültülsün mü?");

    QAbstractButton* pButtonYes = msgBox.addButton(("Küçült"), QMessageBox::YesRole);
    QAbstractButton* pButtonNo = msgBox.addButton(("İptal"), QMessageBox::NoRole);
    QAbstractButton* pButtonReject = msgBox.addButton(("Çıkış"), QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton()==pButtonYes) {
        qDebug("küçült");
        event->ignore();
        this->hide();
    } else if (msgBox.clickedButton()==pButtonReject) {
        qDebug("Çıkış");
        event->accept();
        this->close();
     } else if (msgBox.clickedButton()==pButtonNo) {
        qDebug("iptal");
        event->ignore();
    }
}

void Qtkapat::ZamaniGuncelle()
{
    QDateTime tarih = QDateTime::currentDateTime();
    QString tarihStr = tarih.toString("dd.MM.yyyy hh:mm:ss");
    // arayüz bugünün tarihi
    ui->label_st->setText(tarihStr);
}

void Qtkapat::IslemZamani()
{
    // Belirtilen zaman düğmesi
    if (ui->radioButton_bz->isChecked()) {
        qint64 simdikiZaman = QDateTime::currentSecsSinceEpoch();
        QDateTime secilenZamanS= ui->dateTimeEdit_bz->dateTime();
        qint64 secilenZaman = secilenZamanS.toSecsSinceEpoch() - \
                                (ui->dateTimeEdit_bz->time().second());
        hedef_sure = secilenZaman - simdikiZaman;

      // Belirtilen saat düğmesi
    } else if (ui->radioButton_bs->isChecked()) {
        QTime simdikiSaatS =QTime::currentTime();
        int simdikiSaat = QTime(0,0,0).secsTo(simdikiSaatS);
        int secilenSaat = (ui->timeEdit_bs->time().hour() *3600) + \
                           (ui->timeEdit_bs->time().minute() *60);
        hedef_sure = secilenSaat - simdikiSaat;

      // Belirtilen dakika düğmesi
    } else if (ui->radioButton_dk->isChecked()) {
        hedef_sure = (ui->spinBox_dk->value() * 60);
    }

    if (hedef_sure <= 0) {
        qDebug( "hedef süre :" "%d", hedef_sure);
        QMessageBox::warning(this, "Qtkapat",
                                "Geçmiş ya da hatalı süre!",
                                         QMessageBox::Ok);
    } else {
        ui->pushButton_gb->setEnabled(false);
        ui->pushButton_ip->setEnabled(true);
        zamanlayici->start();
    }
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
                    IslemZamani();
                }

    } else if (ui->radioButton_yb->isChecked()) { // yeniden başlat düğmesi
                if (ui->radioButton_sy->isChecked()){
                    ui->label_us->setText("Sisteminiz yeniden başlatılacak");
                    QProcess::execute(ybaslat_komutu);
                } else {
                    gerisayimStr2 = " yeniden başlatılacak";
                    IslemZamani();
                }

    } else if (ui->radioButton_ok->isChecked()) { // oturumu kapat düğmesi
                if(ui->radioButton_sy->isChecked()){
                   ui->label_us->setText("Oturumunuz kapatılacak.");
                   QProcess::execute(o_kapat_komutu);
                } else {
                    gerisayimStr2 =" oturumunuzu kapatacak";
                    IslemZamani();
                }

    } else if (ui->radioButton_as->isChecked()) { //askıya al düğmesi
                if(ui->radioButton_sy->isChecked()){
                   ui->label_us->setText("Sisteminiz askıya alınacak");
                   QProcess::execute(askiya_al_komutu);
                } else {
                    gerisayimStr2 = " askıya alınacak";
                    IslemZamani();
                }
    } else {
        ui->label_us->setText("Herhangi bir görev seçmediniz.");
        QApplication::processEvents();
        QThread::msleep(1500);
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
    QApplication::processEvents();
    QThread::msleep(1500);
    ui->label_us->setText("Qtkapat -> Görev ve işlem zamanını belirleyin.");
    ui->pushButton_ip->setEnabled(false);
    ui->pushButton_gb->setEnabled(true);
}

void Qtkapat::slot_zamanlayici()
{
    int uyar_spinbox = ui->spinBox_uy->value();
    int uyar_sure = uyar_spinbox * 60;
    QString uyar_ileti = QStringLiteral("Seçtiğiniz görev %1 dk sonra gerçekleştirilecek").arg(uyar_spinbox);

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
        if(hedef_sure > 0)
            trayIcon->showMessage("Qtkapat", uyar_ileti, QSystemTrayIcon::Critical, 5000);
    }

    if (hedef_sure == 0) {
        ui->pushButton_ip->setEnabled(false);
        ui->pushButton_gb->setEnabled(true);
        zamanlayici->stop();

        if (ui->radioButton_kt->isChecked()) {
            QProcess::execute(kapat_komutu);
        } else if (ui->radioButton_yb->isChecked()) {
            QProcess::execute(ybaslat_komutu);
        } else if (ui->radioButton_ok->isChecked()) {
            QProcess::execute(o_kapat_komutu);
        } else if (ui->radioButton_as->isChecked()) {
            QProcess::execute(askiya_al_komutu);
        }
    }
    hedef_sure--;
}
