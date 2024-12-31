#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLabel>

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QTextEdit *textEdit, QWidget *parent = nullptr);

private slots:
    void findNext();
    void findPrevious();

private:
    QTextEdit *textEdit; // 接收区文本编辑框
    QLineEdit *searchLineEdit; // 搜索输入框
    QCheckBox *caseSensitiveCheckBox; // 大小写敏感复选框
    QCheckBox *regexCheckBox; // 正则表达式复选框
    QCheckBox *wholeWordCheckBox; // 全字匹配复选框
    QPushButton *findNextButton; // 查找下一个按钮
    QPushButton *findPreviousButton; // 查找上一个按钮
    QLabel *statusLabel; // 提示信息标签

    void highlightText(const QTextCursor &cursor);
    QTextDocument::FindFlags getFindFlags() const;
    int findTotalMatches() const; // 查找总匹配数
    int findCurrentMatchIndex(const QTextCursor &cursor) const; // 查找当前匹配项的位置
};

#endif // SEARCHDIALOG_H
