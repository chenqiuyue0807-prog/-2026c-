#ifndef HELPWIDGET_H
#define HELPWIDGET_H

#include <QWidget>
#include <QPushButton>

class HelpWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HelpWidget(QWidget *parent = nullptr);
signals:
    void backClicked();
private:
    void setupUI();
};

#endif