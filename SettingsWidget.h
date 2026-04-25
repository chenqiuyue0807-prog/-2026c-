#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QSlider>
#include <QCheckBox>
#include <QComboBox>
#include <QSettings>

class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsWidget(QWidget *parent = nullptr);
signals:
    void backClicked();
    void fullscreenToggled(bool);
    void resolutionChanged(const QSize &);
public slots:
    void applySettings();
    void loadSettings();
private:
    QSlider *m_masterSlider, *m_bgmSlider, *m_sfxSlider;
    QCheckBox *m_fullscreenCheck;
    QComboBox *m_resolutionCombo;
    QSettings *m_settings;
    void setupUI();
};

#endif