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
#include "mytrayicon.h"

#include <QProcess>
#include <QTimer>
#include <QTime>
#include <QDate>
#include <QThread>
#include <QMessageBox>
#include <QCloseEvent>

#if defined(Q_OS_LINUX)
    // Komutlar KDE/Plasma içindir.
    // Yorumlanmışlar genel içindir.
    QString kapat_komutu = "qdbus org.kde.ksmserver /KSMServer logout 0 2 2";
    //QString kapat_komutu = "systemctl poweroff";
    QString ybaslat_komutu = "qdbus org.kde.ksmserver /KSMServer logout 0 1 2";
    //QString ybaslat_komutu = "systemctl restart";
    QString o_kapat_komutu = "qdbus org.kde.ksmserver /KSMServer logout 0 3 3";
    //QString o_kapat_komutu ="loginctl terminate-user fa"; //$USER yerine kullanıcı_adı yazın.
    QString askiya_al_komutu = "qdbus org.kde.Solid.PowerManagement /org/freedesktop/PowerManagement Suspend";
    //QString askiya_al_komutu = "systemctl suspend";
#elif defined(Q_OS_WIN)
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

    QStringList strList;
    strList << "Qtkapat" << "Sistem tepsisinde başlatıldı";
    MyTrayIcon * trayIcon = new MyTrayIcon(strList, this);
    trayIcon->show();

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
    ui->dateTimeEdit_bz->setMinimumDate(QDate::currentDate());
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

void Qtkapat::closeEvent(QCloseEvent *olay)
{
    QMessageBox iltKutu;

    iltKutu.setWindowIcon(QIcon(":/images/shutdown.png"));
    iltKutu.setIcon(QMessageBox::Question);
    iltKutu.setText("Simge durumuna küçültülsün mü?");

    QAbstractButton* pButtonApply = iltKutu.addButton(("Küçült"), QMessageBox::ApplyRole);
    QAbstractButton* pButtonNo = iltKutu.addButton(("İptal"), QMessageBox::NoRole);
    QAbstractButton* pButtonYes = iltKutu.addButton(("Çıkış"), QMessageBox::YesRole);

      iltKutu.exec();

    if (iltKutu.clickedButton()==pButtonApply) {
        qDebug("küçült");
        olay->ignore();
        this->hide();
     } else if (iltKutu.clickedButton()==pButtonYes) {
        qDebug("Çıkış");
        olay->accept();
        this->close();
     } else if (iltKutu.clickedButton()==pButtonNo) {
        qDebug("iptal");
        olay->ignore();
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
        QDateTime seciliZamanS= ui->dateTimeEdit_bz->dateTime();
        qint64 secilenZaman = seciliZamanS.toSecsSinceEpoch() - \
                                (ui->dateTimeEdit_bz->time().second());
        hedef_sure = secilenZaman - simdikiZaman;

      // Belirtilen saat düğmesi
    } else if (ui->radioButton_bs->isChecked()) {
        QTime simdikiSaat =QTime::currentTime();
        int simdikiZaman = QTime(0,0,0).secsTo(simdikiSaat);
        int secilenZaman = (ui->timeEdit_bs->time().hour() *3600) + \
                           (ui->timeEdit_bs->time().minute() *60);
        hedef_sure = secilenZaman - simdikiZaman;

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
    }

    if ((hedef_sure - uyar_sure) == 0) {
        if(hedef_sure >0) {
            QMessageBox msgBox;
            msgBox.setText(uyar_ileti);
            msgBox.setWindowIcon(QIcon(":/images/shutdown.png"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
            msgBox.button(QMessageBox::Ok)->animateClick(5000);

            switch(msgBox.exec())
            {
            case QMessageBox::Ok:
                break;
            case QMessageBox::Cancel:
                ui->pushButton_ip->setEnabled(false);
                ui->pushButton_gb->setEnabled(true);
                zamanlayici->stop();
                ui->label_us->setText("Qtkapat: İşleminiz iptal edildi.");
                QApplication::processEvents();
                QThread::msleep(1500);
                ui->label_us->setText("Qtkapat -> Görev ve işlem zamanını belirleyin.");
                return;
                break;
             default:;
            }
        }
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
