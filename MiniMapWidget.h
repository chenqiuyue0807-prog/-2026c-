#ifndef MINIMAPWIDGET_H
#define MINIMAPWIDGET_H

#include <QWidget>
#include <QTimer>

class GameEngine;

class MiniMapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MiniMapWidget(QWidget *parent = nullptr);
    void setGameEngine(GameEngine *engine);
    void startUpdate();
    void stopUpdate();
protected:
    void paintEvent(QPaintEvent *event) override;
private slots:
    void refresh();
private:
    GameEngine *m_engine = nullptr;
    QTimer *m_timer;
    QPointF worldToMini(const QPointF &wp) const;
    static constexpr qreal SCALE = 0.1;
    static constexpr int MW = 120, MH = 80;
};

#endif