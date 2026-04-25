#ifndef RESULTWIDGET_H
#define RESULTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class ResultWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ResultWidget(QWidget *parent = nullptr);
    void setResult(bool survivorWin, int escaped, int eliminated, int cipher, int rescue, int destroy);
signals:
    void playAgain();
    void backToMenu();
    void quitGame();
private:
    QLabel *m_title, *m_subtitle, *m_stats;
    QPushButton *m_againBtn, *m_menuBtn, *m_quitBtn;
    void setupUI();
    QString bestTitle(bool survivorWin, int escaped, int rescue, int cipher, int eliminated, int destroy);
};

#endif