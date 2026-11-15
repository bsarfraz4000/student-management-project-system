// MainWindow.h - the GUI of the Student Record Management System (Qt Widgets).
#pragma once

#include <QWidget>
#include "StudentStore.h"

class QLineEdit;
class QPushButton;
class QLabel;

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:          // button click handlers (event-driven programming)
    void onAdd();
    void onUpdate();
    void onDelete();
    void onNext();
    void onPrevious();
    void onNew();       // extra: clear the form and go back to "Add" mode
    void onSearch();    // extra: find a student by roll number

private:
    enum class Mode { Add, Browse };

    void buildUi();
    void setMode(Mode mode);
    void showRecord(int index);
    bool readForm(Student& out);       // form -> Student (shows a message if invalid)
    void clearForm();
    void refreshStatus();
    void info(const QString& text);
    void warn(const QString& text);

    StudentStore store_;               // data layer
    Mode         mode_ = Mode::Add;
    int          currentIndex_ = -1;   // index of the record shown in Browse mode

    // Form fields
    QLineEdit *nameEdit_ = nullptr, *rollEdit_ = nullptr, *fatherEdit_ = nullptr,
              *casteEdit_ = nullptr, *districtEdit_ = nullptr, *gpaEdit_ = nullptr,
              *searchEdit_ = nullptr;
    // Buttons
    QPushButton *addBtn_ = nullptr, *updateBtn_ = nullptr, *deleteBtn_ = nullptr,
                *nextBtn_ = nullptr, *prevBtn_ = nullptr, *newBtn_ = nullptr,
                *findBtn_ = nullptr;
    QLabel *statusLabel_ = nullptr;
};
