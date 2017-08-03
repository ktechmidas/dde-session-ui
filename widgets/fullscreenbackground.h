#ifndef FULLSCREENBACKGROUND_H
#define FULLSCREENBACKGROUND_H

#include <QWidget>
#include <QPointer>
#include <QTimer>
#include <QFileSystemWatcher>

class FullscreenBackground : public QWidget
{
    Q_OBJECT

public:
    explicit FullscreenBackground(QWidget *parent = 0);

public slots:
    void setBackground(const QString &file);
    void setBackground(const QPixmap &pixmap);

protected:
    void setContent(QWidget * const w);

private slots:
    void adjustGeometry();
    const QString getBlurImagePath(const QString &path);

private:
    bool eventFilter(QObject *watched, QEvent *event);
    void showEvent(QShowEvent *e);
    void paintEvent(QPaintEvent *e);
    void keyPressEvent(QKeyEvent *e);

private:
    QString m_bgPath;
    QPixmap m_background;
    QPointer<QWidget> m_content;
    QFileSystemWatcher m_blurWatcher;
    QTimer *m_adjustTimer;
};

#endif // FULLSCREENBACKGROUND_H