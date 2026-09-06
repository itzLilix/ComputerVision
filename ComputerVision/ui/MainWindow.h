#pragma once

#include <QMainWindow>
#include <QImage>

class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

    void setImage(QImage img);

private slots:
    void onUploadClicked();
    void onResetClicked();

private:
    QPushButton* uploadButton;
    QPushButton* resetButton;
    QImage uploadedImage;
};