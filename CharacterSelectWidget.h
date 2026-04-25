#ifndef CHARACTERSELECTWIDGET_H
#define CHARACTERSELECTWIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include "GameConfig.h"

class CharacterSelectWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CharacterSelectWidget(QWidget *parent = nullptr);
signals:
    void characterSelected(SurvivorType type);
    void backClicked();
private:
    QButtonGroup *m_cardGroup;
    int m_selectedId = 0;
    SurvivorType m_selectedType = SurvivorType::Doctor;
    void setupUI();
    void createCard(int id, const QString &name, const QString &desc, SurvivorType type);
    QString m_cardNormal, m_cardSelected;
};

#endif