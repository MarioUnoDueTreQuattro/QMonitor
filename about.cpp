#include "about.h"
#include "ui_about.h"
#include <QScreen>
#include <QGuiApplication>
#include <QShowEvent>
#include <QMessageBox>
#include <QSysInfo>
#include <QDebug>
#include <QThread>
#include <QAbstractTextDocumentLayout>
#include <QTimer>
#include "version.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
     ui->setupUi(this);
    this->setWindowTitle("About " + qApp->applicationName());
    this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    // Forza la dimensione fissa in base alla grandezza definita nel file .ui
    //this->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    // ui->textEditInfo->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // ui->textEditInfo->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setupWidgets();
    connect(ui->textEditInfo, &QTextEdit::textChanged, this, &AboutDialog::adjustTextEditHeight);
    // this->adjustSize();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

/**
 * @brief Genera una stringa formattata con le informazioni di sistema dettagliate.
 * @return Una QString contenente tutte le informazioni di sistema.
 */
QString AboutDialog::getSystemInformationString()
{
    // 1. Inizializziamo una QString vuota che conterrà l'output.
    QString infoString;
    // 2. Usiamo QTextStream per scrivere sulla QString in modo efficiente,
    // proprio come faresti con un file o la console, ma in questo caso
    // il "flusso" è la nostra infoString.
    QTextStream out(&infoString);
    // 3. Scriviamo le informazioni una per una.
    // Usiamo '<<' per concatenare il testo e 'endl' (end-line) per andare a capo.
    // out << "=== System Information ===" << Qt::endl;
    // out << "OS Type: " << QSysInfo::productType() << Qt::endl;
    // out << "OS Version: " << QSysInfo::productVersion() << Qt::endl;
    out << "Operating system: " << QSysInfo::prettyProductName() << Qt::endl;
    out << "Kernel: " << QSysInfo::kernelType() << " " << QSysInfo::kernelVersion() << Qt::endl;
    out << "Architecture: " << QSysInfo::currentCpuArchitecture() << " - " << QSysInfo::WordSize << " bits" << Qt::endl;
    // out << "Build Architecture: " << QSysInfo::buildCpuArchitecture() << Qt::endl;
    out << "Logical CPUs: " << QThread::idealThreadCount() << Qt::endl;
    out << "Host name: " << QSysInfo::machineHostName() << Qt::endl;
    out << "Qt version: " << QT_VERSION_STR;
    // out << "Word Size: " << QSysInfo::WordSize << " bits" << Qt::endl;
    // Per il conteggio dei processori
    // 4. La funzione QTextStream 'out' ha scritto tutti i dati nella 'infoString',
    // quindi possiamo semplicemente restituire la stringa.
    return infoString;
}

void AboutDialog::on_pushButtonOk_clicked()
{
    this->close();
}

void AboutDialog::on_pushButtonAboutQt_clicked()
{
    QMessageBox::aboutQt(this);
}

void AboutDialog::adjustTextEditColor()
{
    //QPalette palette = ui->textEditInfo->palette();
    //QPalette textEditPalette = ui->textEditInfo->palette();
    //    // 1. Ottieni il pennello corretto per lo sfondo di un campo di testo: QPalette::Base.
    //    // Usiamo la palette del QTextEdit stesso come riferimento.
    // QBrush defaultBaseBrush = textEditPalette.brush(QPalette::Normal, QPalette::Base);
    // textEditPalette.setBrush(QPalette::Active, QPalette::Base, defaultBaseBrush);
    // textEditPalette.setBrush(QPalette::Inactive, QPalette::Base, defaultBaseBrush);
    // textEditPalette.setBrush(QPalette::Disabled, QPalette::Base, defaultBaseBrush);
    //QColor defaultTextColor = textEditPalette.color(QPalette::Normal, QPalette::Text);
    // textEditPalette.setColor(QPalette::Active, QPalette::Text, defaultTextColor);
    // textEditPalette.setColor(QPalette::Inactive, QPalette::Text, defaultTextColor);
    //    // 3. Applica la palette modificata al widget
    // ui->textEditInfo->setPalette(textEditPalette);
    // ui->textEditInfo->setAutoFillBackground(true);
    QColor windowColor = this->palette().color(QPalette::Window);
    // 2. Converti il colore in una stringa esadecimale (#RRGGBB)
    QString hexColor = windowColor.name(); // Esempio: "#202020"
    // 3. Applica QSS per forzare il background-color
    ui->textEditInfo->setStyleSheet(
        QString("QTextEdit { background-color: %1; }")
        //QString("QTextEdit { background-color: %1; border: 1px solid %2; }")
        .arg(hexColor) // Imposta lo sfondo del QTextEdit al colore della finestra
        //.arg(hexColor) // Opzionalmente, rimuove anche il bordo di focus/default
    );
}

void AboutDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    centerOnParent(); // Primo posizionamento preventivo
}

void AboutDialog::setupWidgets()
{
    //QPixmap iconPix = QPixmap(":/img/img/icons8-play-96.png").scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    //QPixmap iconPix = QPixmap(":/img/img/icons8-play-96.png");
    QPixmap iconPix = QIcon::fromTheme("QMonitor").pixmap(128, 128);
    // Se l'icona non viene trovata nel sistema, iconPix sarà vuota (.isNull() == true)
    if (iconPix.isNull())
    {
        qWarning() << "Attenzione: Icona non trovata nei temi di sistema!";
    }
    ui->labelIcon->setPixmap(iconPix);
    QFont appFont = ui->labelAppName->font();
    appFont.setBold(true);
    appFont.setPixelSize(24);
    ui->labelAppName->setFont(appFont);
    ui->labelAppName->setText(qApp->applicationName());
    QString sIsDebug = "";
#ifdef QT_DEBUG
    sIsDebug = " Debug ";
#else
    sIsDebug = " Release ";
#endif
    QString sVersion = "<h3><p style=\"font-weight: bold; \">Version ";
    sVersion = sVersion + APP_VERSION;
    sVersion = sVersion + sIsDebug + "</p></h3>";
    sVersion = sVersion + "<p>Build date and time: " + __DATE__ + " at " + __TIME__ + "</p>";
    ui->labelAppVersion->setText(sVersion);
    ui->labelAppCopyright->setText("<p>Copyright &copy; 2026 Andrea G.<br>"
                                   "All rights reserved.</p>"
        /* "<p>Visit our website: <a href='https://www.example.com'>www.example.com</a></p>"*/);
    //ui->textEditInfo->hide();
    ui->textEditInfo->setText(getSystemInformationString());
    // Imposta entrambe le barre di scorrimento su 'Mai'
    // ui->textEditInfo->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    QTimer::singleShot(100, this, [this]()
    {
        this->adjustTextEditHeight();
        this->adjustTextEditColor();
        adjustSize(); // Ricalcola la dimensione del QDialog in base al nuovo QTextEdit
        //centerOnParent(); // Posiziona correttamente al centro
        // Potrebbe essere ancora utile chiamare processEvents() subito prima, ma
        // QTimer::singleShot(0, ...) è generalmente sufficiente.
        //this->setFixedSize(this->size());
        centerOnScreen(this);
    });
}

void AboutDialog::adjustTextEditHeight()
{
    // 1. Ottieni il documento del QTextEdit
    // ui->textEditInfo->update();
    // QCoreApplication::processEvents();
    QTextDocument *doc = ui->textEditInfo->document();
    // 2. Calcola l'altezza necessaria per l'intero contenuto formattato (in pixel)
    // Usiamo documentLayout()->documentSize().height() per l'altezza del testo.
    qreal textHeight = doc->documentLayout()->documentSize().height();
    // 3. Aggiungi il margine del riquadro (frame) del widget.
    // Il frameWidth() è la larghezza del bordo attorno al widget.
    int frameHeight = ui->textEditInfo->frameWidth() * 2;
    // 4. Calcola l'altezza totale: altezza del testo + margini.
    int newHeight = qRound(textHeight) + frameHeight;
    //qDebug() << "newHeight:" << newHeight;
    // 5. Imposta l'altezza desiderata
    ui->textEditInfo->setFixedHeight(newHeight);
    ui->textEditInfo->setMinimumSize(ui->textEditInfo->width() + 10, newHeight);
}

void AboutDialog::centerOnScreen(QDialog *dialog)
{
    if (!dialog)
        return;

    const QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    const QRect available = screen->availableGeometry();

    const int x = available.x() + (available.width()  - dialog->frameGeometry().width())  / 2;
    const int y = available.y() + (available.height() - dialog->frameGeometry().height()) / 2;

    dialog->move(x, y);
}

void AboutDialog::centerOnParent()
{
    QWidget *parent = parentWidget();
    if (parent)
    {
        // frameGeometry() include anche i bordi e la barra del titolo della finestra principale
        QRect parentGeo = parent->frameGeometry();
        int x = parentGeo.x() + (parentGeo.width() - width()) / 2;
        int y = parentGeo.y() + (parentGeo.height() - height()) / 2;
        move(x, y);
    }
    else
    {
        // Fallback sul centro dello schermo attivo se non c'è un parent
        QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
        if (!screen)
        {
            screen = QGuiApplication::primaryScreen();
        }
        if (screen)
        {
            QRect screenGeo = screen->availableGeometry();
            int x = screenGeo.x() + (screenGeo.width() - width()) / 2;
            int y = screenGeo.y() + (screenGeo.height() - height()) / 2;
            move(x, y);
        }
    }
}