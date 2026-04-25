#ifndef LOADINGWIDGET_H
#define LOADINGWIDGET_H

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>

class LoadingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LoadingWidget(QWidget *parent = nullptr);
signals:
    void loadingFinished();
public slots:
    void startLoading();
private:
    QProgressBar *m_progressBar;
    QLabel *m_tipLabel;
    QTimer *m_timer;
    int m_progressValue = 0;
    void setupUI();
    QString getRandomTip() const;
};

#endif