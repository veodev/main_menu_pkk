#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    void powerSingleChannelUmu(bool isPower = false);
    void powerMultiChannelUmu(bool isPower = false);

protected:
    void keyPressEvent(QKeyEvent * ke);

private:
    void searchUpdatesOnFlash();
    bool isUmuFound();

private slots:
    void onBrigthnessChanged(int value);
    void onUpdateTime();
    void on_aviconDbMultiChannelButton_released();
    void on_aviconDbSingleChannelButton_released();
    void on_powerOffButton_released();
    void on_okPushButton_released();
    void on_cancelPushButton_released();
    void updateProgressBar();
    void onPowerOffTimerTimeOut();
    void onPressedKey(int key);
    void isFlashMount();
    void onWrite(qint64 writeBytes);

    void on_updateSoftButton_released();
    void on_noUpdatePushButton_released();
    void on_updatePushButton_released();

    void returnAviconDbSingleButtonText();

signals:
    void closeApplication();
    void pressedKey(int key);

private:
    Ui::Widget *ui;

    QTimer * _timeUpdateTimer;
    QTimer * _powerOffTimer;
    QTimer * _checkUsbFlashAvailable;
    QString _flashDirPath;
    int _writtenBytes;
};

#endif // WIDGET_H
