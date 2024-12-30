#include "searchdialog.h"

SearchDialog::SearchDialog(QTextEdit *textEdit, QWidget *parent)
    : QDialog(parent), textEdit(textEdit)
{
    setWindowTitle("Search");
    setModal(false); // 非模态对话框

    // 搜索输入框
    searchLineEdit = new QLineEdit(this);

    // 复选框
    caseSensitiveCheckBox = new QCheckBox("Case Sensitive", this);
    regexCheckBox = new QCheckBox("Regular Expression", this);

    // 按钮
    findNextButton = new QPushButton("Find Next", this);
    findPreviousButton = new QPushButton("Find Previous", this);

    // 布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(searchLineEdit);
    mainLayout->addWidget(caseSensitiveCheckBox);
    mainLayout->addWidget(regexCheckBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(findNextButton);
    buttonLayout->addWidget(findPreviousButton);
    mainLayout->addLayout(buttonLayout);

    // 连接信号和槽
    connect(findNextButton, &QPushButton::clicked, this, &SearchDialog::findNext);
    connect(findPreviousButton, &QPushButton::clicked, this, &SearchDialog::findPrevious);
}

void SearchDialog::findNext()
{
    QString searchText = searchLineEdit->text();
    if (searchText.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter a search term.");
        return;
    }

    QTextDocument::FindFlags flags;
    if (caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }

    QTextCursor cursor = textEdit->textCursor();
    if (regexCheckBox->isChecked()) {
        QRegExp regex(searchText, caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
        cursor = textEdit->document()->find(regex, cursor, flags);
    } else {
        cursor = textEdit->document()->find(searchText, cursor, flags);
    }

    if (cursor.isNull()) {
        QMessageBox::information(this, "Search", "No more matches found.");
    } else {
        highlightText(cursor);
    }
}

void SearchDialog::findPrevious()
{
    QString searchText = searchLineEdit->text();
    if (searchText.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter a search term.");
        return;
    }

    QTextDocument::FindFlags flags = QTextDocument::FindBackward;
    if (caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }

    QTextCursor cursor = textEdit->textCursor();
    if (regexCheckBox->isChecked()) {
        QRegExp regex(searchText, caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
        cursor = textEdit->document()->find(regex, cursor, flags);
    } else {
        cursor = textEdit->document()->find(searchText, cursor, flags);
    }

    if (cursor.isNull()) {
        QMessageBox::information(this, "Search", "No more matches found.");
    } else {
        highlightText(cursor);
    }
}

void SearchDialog::highlightText(const QTextCursor &cursor)
{
    QTextEdit::ExtraSelection extra;
    extra.cursor = cursor;
    extra.format.setBackground(Qt::yellow); // 高亮背景色为黄色
    textEdit->setTextCursor(cursor);
    textEdit->setExtraSelections(QList<QTextEdit::ExtraSelection>() << extra);
}
