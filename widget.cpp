#include <QKeyEvent>
#include <QTextStream>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QFile>

#include "widget.h"
#include "ui_widget.h"

#define F1_BUTTON 16777264
#define F2_BUTTON 16777265
#define F3_BUTTON 16777266
#define POWER_BUTTON 16777399
#define UPDATE_TIME_INTERVAL 1000
#define UPDATE_FLASH_STATUS_TIMER 1000
#define POWER_OFF_TIMER 10000

const QString buttonStyle = "QPushButton {color: #000000; border-radius: 5px; border: 2px solid #055cab; background-color: #FFFFFF;}"
    "QPushButton:pressed {color: #FFFFFF; border-radius: 5px; border: 2px solid #055cab; background-color: #055cab;}"
    "QPushButton:checked {color: #FFFFFF; border-radius: 5px; border: 2px solid #055cab; background-color: #055cab;}"
    "QPushButton:disabled {color: #b0b0b0; border-radius: 5px; border: 2px solid #b0b0b0; background-color: #FFFFFF;}";

const QString umuNotFoundStyle = "QPushButton {color: red; border-radius: 5px; border: 2px solid #055cab; background-color: #FFFFFF;}";

const QString nameSingleChannelProgram = "avicondbhs";
const QString nameMultiChannelProgram = "avicondb";


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , _flashDirPath("")
    , _writtenBytes(0)
{
    ui->setupUi(this);
    ui->updateSoftDialogWidget->hide();
    ui->powerDialogWidget->hide();
    ui->copyFilesProgressBar->hide();
    ui->aviconDbMultiChannelButton->setStyleSheet(buttonStyle);
    ui->aviconDbSingleChannelButton->setStyleSheet(buttonStyle);
    ui->updateSoftButton->setStyleSheet(buttonStyle);
    ui->powerOffButton->setStyleSheet(buttonStyle);
    ui->okPushButton->setStyleSheet(buttonStyle);
    ui->cancelPushButton->setStyleSheet(buttonStyle);
    ui->updatePushButton->setStyleSheet(buttonStyle);
    ui->noUpdatePushButton->setStyleSheet(buttonStyle);
    ui->updateSoftButton->setDisabled(true);

    _timeUpdateTimer = new QTimer(this);
    _timeUpdateTimer->start(UPDATE_TIME_INTERVAL);
    connect(_timeUpdateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTime()));

    _powerOffTimer = new QTimer(this);
    connect(_timeUpdateTimer, SIGNAL(timeout()), this, SLOT(updateProgressBar()));
    connect(_powerOffTimer, SIGNAL(timeout()), this, SLOT(onPowerOffTimerTimeOut()));

    _checkUsbFlashAvailable = new QTimer(this);
    _checkUsbFlashAvailable->start(UPDATE_FLASH_STATUS_TIMER);
    connect(_checkUsbFlashAvailable, SIGNAL(timeout()), this, SLOT(isFlashMount()));

    connect(this, SIGNAL(pressedKey(int)), this, SLOT(onPressedKey(int)));

    onBrigthnessChanged(50);
    powerSingleChannelUmu();
    powerMultiChannelUmu();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::powerSingleChannelUmu(bool isPower)
{
    QFile file("/sys/class/leds/pkkled1/brightness");
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    out << isPower;
}

void Widget::powerMultiChannelUmu(bool isPower)
{
    QFile file("/sys/class/leds/pkkled0/brightness");
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    out << isPower;
}

void Widget::keyPressEvent(QKeyEvent *ke)
{
    switch (ke->key()) {
    case F1_BUTTON:
        emit pressedKey(F1_BUTTON);
        break;
    case F2_BUTTON:
        emit pressedKey(F2_BUTTON);
        break;
    case F3_BUTTON:
        emit pressedKey(F3_BUTTON);
        break;
    case POWER_BUTTON:
        emit pressedKey(POWER_BUTTON);
        break;
    default:
        break;
    }
}

void Widget::searchUpdatesOnFlash()
{
    QDir dir(_flashDirPath);
    if (dir.exists()) {
        dir.setFilter(QDir::Files);
        QStringList nameFilters;
        nameFilters << nameMultiChannelProgram << nameSingleChannelProgram;
        QStringList filesList = dir.entryList(nameFilters);
        if (filesList.empty() == false) {
            ui->listWidget->clear();
            ui->listWidget->addItems(filesList);
            ui->resultSearchInfoLabel->setText(QString("Found ") + QString::number(filesList.count()) + QString(" files to update."));
            ui->updatePushButton->setEnabled(true);
        }
        else {
            ui->listWidget->clear();
            ui->resultSearchInfoLabel->setText(QString("No files for update!"));
            ui->updatePushButton->setDisabled(false);
        }
    }
}

bool Widget::isUmuFound()
{
    QFile file("/tmp/umu_dump.tmp");
    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        if (file.readAll().isEmpty() == false) {
            file.close();
            return true;
        }
    }
    ui->aviconDbSingleChannelButton->setStyleSheet(umuNotFoundStyle);
    ui->aviconDbSingleChannelButton->setText("UMU not found!");
    QTimer::singleShot(3000, this, SLOT(returnAviconDbSingleButtonText()));
    return false;
}

void Widget::onBrigthnessChanged(int value)
{
    if (value <= 100 && value >= 0) {
        QFile file("/sys/class/backlight/pkk/brightness");
        file.open(QIODevice::WriteOnly);
        QTextStream out(&file);
        out << value;
    }
}

void Widget::onUpdateTime()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    int currentHours = currentDateTime.time().hour();
    int currentMinutes = currentDateTime.time().minute();
    int currentSeconds = currentDateTime.time().second();

    (currentHours < 10) ? ui->hoursLcdNumber->display("0" + QString::number(currentHours)) : ui->hoursLcdNumber->display(currentHours);
    (currentMinutes < 10) ? ui->minutesLcdNumber->display("0" + QString::number(currentMinutes)) : ui->minutesLcdNumber->display(currentMinutes);
    (currentSeconds < 10) ? ui->secLcdNumber->display("0" + QString::number(currentSeconds)) : ui->secLcdNumber->display(currentSeconds);

    ui->dateLabel->setText(currentDateTime.date().toString("dd MMM yyyy"));
}

void Widget::on_aviconDbMultiChannelButton_released()
{
    QDir pkkAviconDbDir("/usr/local/avicon-db");
    if (pkkAviconDbDir.entryList().contains(nameMultiChannelProgram) == false) {
        return;
    }
    powerMultiChannelUmu(true);
    QProcess * process = new QProcess(this);
    bool ifStart = process->startDetached("/usr/local/scripts/avicondb_script.sh");
    if (ifStart == true) {
        this->deleteLater();
    }
}

void Widget::on_aviconDbSingleChannelButton_released()
{
    QDir pkkAviconDbHsDir("/usr/local/avicon-dbhs");
    if (pkkAviconDbHsDir.entryList().contains(nameSingleChannelProgram) == false) {
        return;
    }
    if (isUmuFound() == false) {
        return;
    }
    powerSingleChannelUmu(true);
    QProcess * process = new QProcess(this);
    bool ifStart = process->startDetached("/usr/local/scripts/avicondbhs_script.sh");
    if (ifStart == true) {
        this->deleteLater();
    }
}

void Widget::on_powerOffButton_released()
{
    disconnect(this, SIGNAL(pressedKey(int)), this, SLOT(onPressedKey(int)));
    _powerOffTimer->start(POWER_OFF_TIMER);
    _checkUsbFlashAvailable->stop();
    ui->progressBar->setMaximum(ui->progressBar->maximum());
    ui->widget->hide();
    ui->powerDialogWidget->show();

}

void Widget::on_okPushButton_released()
{
    ui->okPushButton->setChecked(true);
    onPowerOffTimerTimeOut();
}

void Widget::on_cancelPushButton_released()
{
    disconnect(this, SIGNAL(pressedKey(int)), this, SLOT(onPressedKey(int)));
    connect(this, SIGNAL(pressedKey(int)), this, SLOT(onPressedKey(int)));
    _powerOffTimer->stop();
    _checkUsbFlashAvailable->start(UPDATE_FLASH_STATUS_TIMER);
    ui->widget->show();
    ui->powerDialogWidget->hide();
}

void Widget::updateProgressBar()
{
    ui->progressBar->setValue(_powerOffTimer->remainingTime() / 1000);
}

void Widget::onPowerOffTimerTimeOut()
{
    QProcess * process = new QProcess(this);
    process->start("poweroff");
}

void Widget::onPressedKey(int key)
{
    switch (key) {
    case F1_BUTTON:
        on_aviconDbMultiChannelButton_released();
        break;
    case F2_BUTTON:
        on_aviconDbSingleChannelButton_released();
        break;
    case F3_BUTTON:
        break;
    case POWER_BUTTON:
        on_powerOffButton_released();
        break;
    default:
        break;
    }
}

void Widget::isFlashMount()
{
    QProcess process(this);
    process.start("sh");
    process.waitForStarted();
    process.write("df | grep sdb | tr -s \" \" \" \" | cut -d \" \" -f1");
    process.closeWriteChannel();
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();

    if (output.isEmpty() == true) {
        ui->updateSoftButton->setDisabled(true);
        _flashDirPath = "";
    }
    else {
        ui->updateSoftButton->setEnabled(true);
        QProcess process2(this);
        process2.start("sh");
        process2.waitForStarted();
        process2.write("df | grep sdb | tr -s \" \" \" \" | cut -d \" \" -f6");
        process2.closeWriteChannel();
        process2.waitForFinished();
        QByteArray output2 = process2.readAllStandardOutput();
        output2 = output2.remove(output2.size() - 1, 1);
        _flashDirPath = output2;
    }
}

void Widget::onWrite(qint64 writeBytes)
{
    _writtenBytes += writeBytes;
    qDebug() << _writtenBytes;
    ui->copyFilesProgressBar->setValue(_writtenBytes / 1024);
}

void Widget::on_updateSoftButton_released()
{
    ui->widget->hide();
    ui->powerDialogWidget->hide();
    ui->updatePushButton->setText("Update");
    ui->noUpdatePushButton->setText("Cancel");
    ui->updatePushButton->show();
    ui->noUpdatePushButton->show();
    ui->updateSoftDialogWidget->show();
    searchUpdatesOnFlash();
}

void Widget::on_noUpdatePushButton_released()
{
    ui->widget->show();
    ui->powerDialogWidget->hide();
    ui->updateSoftDialogWidget->hide();
    ui->noUpdatePushButton->setText("CANCEL");
}

void Widget::on_updatePushButton_released()
{
    ui->updatePushButton->hide();

    QDir flashDir(_flashDirPath);
    QStringList flashFilesList = flashDir.entryList();

    QDir pkkAviconDbDir("/usr/local/avicon-db");
    QStringList pkkAviconDbDirFilesList = pkkAviconDbDir.entryList();

    QDir pkkAviconDbHsDir("/usr/local/avicon-dbhs");
    QStringList pkkAviconDbHsDirFilesList = pkkAviconDbHsDir.entryList();

    if (flashFilesList.contains(nameMultiChannelProgram)) {
        QFile aviconDbFileOnFlash(_flashDirPath + "/" + nameMultiChannelProgram);
        QFile aviconDbFileOnPkk("/usr/local/avicon-db/avicondb_");
        if (pkkAviconDbDirFilesList.contains(nameMultiChannelProgram)) {
            pkkAviconDbDir.remove(nameMultiChannelProgram);
        }
        ui->copyFilesProgressBar->setMaximum(QFileInfo(aviconDbFileOnFlash).size() / 1024);
        _writtenBytes = 0;
        connect(&aviconDbFileOnPkk, SIGNAL(bytesWritten(qint64)), this, SLOT(onWrite(qint64)));
        aviconDbFileOnFlash.copy(aviconDbFileOnPkk.fileName());
        aviconDbFileOnPkk.rename("/usr/local/avicon-db/avicondb");
    }

    if (flashFilesList.contains(nameSingleChannelProgram)) {
        QFile aviconDbHsFileOnFlash(_flashDirPath + "/" + nameSingleChannelProgram);
        QFile aviconDbHsFileOnPkk("/usr/local/avicon-dbhs/avicondbhs_");
        if (pkkAviconDbHsDirFilesList.contains(nameSingleChannelProgram)) {
            pkkAviconDbHsDir.remove(nameSingleChannelProgram);
        }
        ui->copyFilesProgressBar->setMaximum(QFileInfo(aviconDbHsFileOnFlash).size() / 1024);
        _writtenBytes = 0;
        connect(&aviconDbHsFileOnPkk, SIGNAL(bytesWritten(qint64)), this, SLOT(onWrite(qint64)));
        aviconDbHsFileOnFlash.copy(aviconDbHsFileOnPkk.fileName());
        aviconDbHsFileOnPkk.rename("/usr/local/avicon-dbhs/avicondbhs");
    }
    ui->resultSearchInfoLabel->setText("Update finished!");
    ui->noUpdatePushButton->setText("OK");
}

void Widget::returnAviconDbSingleButtonText()
{
    ui->aviconDbSingleChannelButton->setStyleSheet(buttonStyle);
    ui->aviconDbSingleChannelButton->setText("SINGLECHANNEL");
}
