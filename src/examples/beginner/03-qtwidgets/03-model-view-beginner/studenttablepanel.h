#pragma once

#include <QStringList>
#include <QWidget>

class QStandardItemModel;
class QTableView;

// 学生成绩表格面板：QStandardItemModel + QTableView
class StudentTablePanel : public QWidget
{
    Q_OBJECT

public:
    explicit StudentTablePanel(QWidget *parent = nullptr);

    /// 获取所有学生的姓名列表
    QStringList getStudentNames() const;

    /// 添加一名学生
    void addStudent(const QString &name, int chinese, int math, int english);

    QStandardItemModel *model() const;

signals:
    void namesChanged();

private:
    void addStudentRaw(const QString &name, int chinese, int math, int english);
    void showContextMenu(const QPoint &pos);

    QStandardItemModel *m_model = nullptr;
    QTableView *m_tableView = nullptr;
};
