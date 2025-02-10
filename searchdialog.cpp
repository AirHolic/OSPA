#include "searchdialog.h"

SearchDialog::SearchDialog(const QString &portName, QTextEdit *textEdit, QWidget *parent)
    : QDialog(parent), textEdit(textEdit)
{
    // 窗口标题中仅 Search 部分需要支持翻译，portName 保持原样
    setWindowTitle(portName + " - " + tr("Search"));
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setModal(false); // 非模态对话框
    //setWindowFlag(Qt::FramelessWindowHint); // 无边框窗口

    // 搜索输入框
    searchLineEdit = new QLineEdit(this);

    // 复选框
    caseSensitiveCheckBox = new QCheckBox(tr("Case Sensitive"), this);
    regexCheckBox = new QCheckBox(tr("Regular Expression"), this);
    wholeWordCheckBox = new QCheckBox(tr("Whole Word"), this);

    // 按钮
    findNextButton = new QPushButton(tr("Find Next"), this);
    findPreviousButton = new QPushButton(tr("Find Previous"), this);

    // 提示信息标签
    statusLabel = new QLabel(tr("0 of 0"), this);

    // 布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(searchLineEdit);
    mainLayout->addWidget(caseSensitiveCheckBox);
    mainLayout->addWidget(regexCheckBox);
    mainLayout->addWidget(wholeWordCheckBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(findNextButton);
    buttonLayout->addWidget(findPreviousButton);
    mainLayout->addLayout(buttonLayout);

    mainLayout->addWidget(statusLabel); // 添加提示信息标签

    // 连接信号和槽
    connect(findNextButton, &QPushButton::clicked, this, &SearchDialog::findNext);
    connect(findPreviousButton, &QPushButton::clicked, this, &SearchDialog::findPrevious);
}

void SearchDialog::closeEvent(QCloseEvent *)
{
    setShowFlag(false);
}

void SearchDialog::reject()
{
    setShowFlag(false);
    QDialog::reject();
}

bool SearchDialog::getShowFlag()
{
    return showFlag;
}

void SearchDialog::setShowFlag(bool flag)
{
    showFlag = flag;
}

void SearchDialog::findNext()
{
    QString searchText = searchLineEdit->text();
    if (searchText.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please enter a search term."));
        return;
    }

    QTextCursor cursor = textEdit->textCursor();
    QTextDocument::FindFlags flags = getFindFlags();

    if (regexCheckBox->isChecked()) {
        QRegExp regex(searchText, caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
        cursor = textEdit->document()->find(regex, cursor, flags);
    } else if (wholeWordCheckBox->isChecked()) {
        cursor = textEdit->document()->find(searchText, cursor, flags | QTextDocument::FindWholeWords);
    } else {
        cursor = textEdit->document()->find(searchText, cursor, flags);
    }

    if (cursor.isNull()) {
        // 搜索到最后一条，从头开始
        cursor = textEdit->textCursor();
        cursor.movePosition(QTextCursor::Start);
        if (regexCheckBox->isChecked()) {
            QRegExp regex(searchText, caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
            cursor = textEdit->document()->find(regex, cursor, flags);
        } else if (wholeWordCheckBox->isChecked()) {
            cursor = textEdit->document()->find(searchText, cursor, flags | QTextDocument::FindWholeWords);
        } else {
            cursor = textEdit->document()->find(searchText, cursor, flags);
        }

        if (cursor.isNull()) {
            QMessageBox::information(this, tr("Search"), tr("No matches found."));
            statusLabel->setText(tr("0 of 0"));
        } else {
            highlightText(cursor);
            int totalMatches = findTotalMatches();
            int currentMatchIndex = findCurrentMatchIndex(cursor);
            statusLabel->setText(QString("%1 of %2").arg(currentMatchIndex + 1).arg(totalMatches));
        }
    } else {
        highlightText(cursor);
        int totalMatches = findTotalMatches();
        int currentMatchIndex = findCurrentMatchIndex(cursor);
        statusLabel->setText(QString("%1 of %2").arg(currentMatchIndex + 1).arg(totalMatches));
    }
}

void SearchDialog::findPrevious()
{
    QString searchText = searchLineEdit->text();
    if (searchText.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please enter a search term."));
        return;
    }

    QTextCursor cursor = textEdit->textCursor();
    QTextDocument::FindFlags flags = getFindFlags() | QTextDocument::FindBackward;

    if (regexCheckBox->isChecked()) {
        QRegExp regex(searchText, caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
        cursor = textEdit->document()->find(regex, cursor, flags);
    } else if (wholeWordCheckBox->isChecked()) {
        cursor = textEdit->document()->find(searchText, cursor, flags | QTextDocument::FindWholeWords);
    } else {
        cursor = textEdit->document()->find(searchText, cursor, flags);
    }

    if (cursor.isNull()) {
        // 搜索到第一条，从末尾开始
        cursor = textEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        if (regexCheckBox->isChecked()) {
            QRegExp regex(searchText, caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
            cursor = textEdit->document()->find(regex, cursor, flags);
        } else if (wholeWordCheckBox->isChecked()) {
            cursor = textEdit->document()->find(searchText, cursor, flags | QTextDocument::FindWholeWords);
        } else {
            cursor = textEdit->document()->find(searchText, cursor, flags);
        }

        if (cursor.isNull()) {
            QMessageBox::information(this, tr("Search"), tr("No matches found."));
            statusLabel->setText(tr("0 of 0"));
        } else {
            highlightText(cursor);
            int totalMatches = findTotalMatches();
            int currentMatchIndex = findCurrentMatchIndex(cursor);
            statusLabel->setText(QString("%1 of %2").arg(currentMatchIndex + 1).arg(totalMatches));
        }
    } else {
        highlightText(cursor);
        int totalMatches = findTotalMatches();
        int currentMatchIndex = findCurrentMatchIndex(cursor);
        statusLabel->setText(QString("%1 of %2").arg(currentMatchIndex + 1).arg(totalMatches));
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

QTextDocument::FindFlags SearchDialog::getFindFlags() const
{
    QTextDocument::FindFlags flags;
    if (caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    return flags;
}

int SearchDialog::findTotalMatches() const
{
    QString searchText = searchLineEdit->text();
    if (searchText.isEmpty()) {
        return 0;
    }

    QTextDocument::FindFlags flags = getFindFlags();
    QTextCursor cursor(textEdit->document());
    int totalMatches = 0;

    if (regexCheckBox->isChecked()) {
        QRegExp regex(searchText, caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
        while (!(cursor = textEdit->document()->find(regex, cursor, flags)).isNull()) {
            totalMatches++;
        }
    } else if (wholeWordCheckBox->isChecked()) {
        while (!(cursor = textEdit->document()->find(searchText, cursor, flags | QTextDocument::FindWholeWords)).isNull()) {
            totalMatches++;
        }
    } else {
        while (!(cursor = textEdit->document()->find(searchText, cursor, flags)).isNull()) {
            totalMatches++;
        }
    }

    return totalMatches;
}

int SearchDialog::findCurrentMatchIndex(const QTextCursor &currentCursor) const
{
    QString searchText = searchLineEdit->text();
    if (searchText.isEmpty()) {
        return -1;
    }

    QTextDocument::FindFlags flags = getFindFlags();
    QTextCursor cursor(textEdit->document());
    int currentMatchIndex = -1;
    int index = 0;

    if (regexCheckBox->isChecked()) {
        QRegExp regex(searchText, caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
        while (!(cursor = textEdit->document()->find(regex, cursor, flags)).isNull()) {
            if (cursor == currentCursor) {
                currentMatchIndex = index;
                break;
            }
            index++;
        }
    } else if (wholeWordCheckBox->isChecked()) {
        while (!(cursor = textEdit->document()->find(searchText, cursor, flags | QTextDocument::FindWholeWords)).isNull()) {
            if (cursor == currentCursor) {
                currentMatchIndex = index;
                break;
            }
            index++;
        }
    } else {
        while (!(cursor = textEdit->document()->find(searchText, cursor, flags)).isNull()) {
            if (cursor == currentCursor) {
                currentMatchIndex = index;
                break;
            }
            index++;
        }
    }

    return currentMatchIndex;
}
