#pragma once

#include <QStringList>
#include <QWidget>

class QStringListModel;
class QListView;

// 姓名列表面板：QStringListModel + QListView
class NameListPanel : public QWidget
{
    Q_OBJECT

public:
    explicit NameListPanel(QWidget *parent = nullptr);

    /// 刷新姓名列表（从外部同步过来）
    void setNames(const QStringList &names);

    QStringListModel *model() const;

signals:
    void addStudentRequested(const QString &name);
    void removeStudentRequested(int row);
    void nameEdited(int row, const QString &newName);

private:
    QStringListModel *m_model = nullptr;
    QListView *m_listView = nullptr;
};
