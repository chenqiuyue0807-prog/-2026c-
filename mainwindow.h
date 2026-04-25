#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include "GameScene.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void setupUI();   // === 第3步新增 ===

    GameScene *m_scene;
    QGraphicsView *m_view;

    // === 第3步新增：UI控件 ===
    QWidget *m_centralWidget;
    QLabel *m_phaseLabel;
    QLabel *m_cipherLabel;
    QLabel *m_timerLabel;
    QProgressBar *m_interactProgressBar;
};

#endif // MAINWINDOW_H