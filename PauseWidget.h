#ifndef PAUSEWIDGET_H
#define PAUSEWIDGET_H

#include <QWidget>
#include <QPushButton>

class PauseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PauseWidget(QWidget *parent = nullptr);
signals:
    void resumeGame();
    void restartGame();
    void backToMainMenu();
protected:
    void keyPressEvent(QKeyEvent *event) override;
private:
    void setupUI();
};

#endif