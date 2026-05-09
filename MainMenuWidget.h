#ifndef MAINMENUWIDGET_H
#define MAINMENUWIDGET_H
#include "utils/AudioManager.h"

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QMediaPlayer>

class MainMenuWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainMenuWidget(QWidget *parent = nullptr);
signals:
    void startGameClicked();
    void settingsClicked();
    void helpClicked();
    void quitClicked();
protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
private:
    QLabel *m_titleLabel;
    QPushButton *m_startBtn, *m_settingsBtn, *m_helpBtn, *m_quitBtn;
    QMediaPlayer *m_bgmPlayer;
    void setupUI();
    void setupStyle();
};

#endif