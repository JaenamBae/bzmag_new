#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr)
        : QDialog(parent) {
        setWindowTitle("About");

        // 레이아웃 설정
        QVBoxLayout* layout = new QVBoxLayout(this);

        // 내용 추가
        QLabel* titleLabel = new QLabel("<b>About bzMagEditor</b>", this);
        titleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLabel);

        QLabel* poweredLabel = new QLabel("Powered by <b>Gmsh</b> & <b>GetDP</b>", this);
        poweredLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(poweredLabel);

        QLabel* authorLabel = new QLabel("Made by <b>Jaenam Bae</b>", this);
        authorLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(authorLabel);

        QLabel* contactLabel = new QLabel("Contact: <a href=\"mailto:jaenam@dongyang.ac.kr\">jaenam@dongyang.ac.kr</a>", this);
        contactLabel->setAlignment(Qt::AlignCenter);
        contactLabel->setOpenExternalLinks(true); // 이메일 링크 클릭 가능
        layout->addWidget(contactLabel);

        // 닫기 버튼
        QPushButton* closeButton = new QPushButton("Close", this);
        connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
        layout->addWidget(closeButton);
    }
};
