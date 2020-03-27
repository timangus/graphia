#include "opengldebuglogger.h"

#include <QOpenGLDebugLogger>
#include <QOpenGLContext>
#include <QDebug>

OpenGLDebugLogger::OpenGLDebugLogger(QObject* parent) :
    QObject(parent)
{
    _debugLevel = qEnvironmentVariableIntValue("OPENGL_DEBUG");
    if(_debugLevel != 0)
    {
        _logger = new QOpenGLDebugLogger(this);
        if(_logger->initialize())
        {
            const QList<QOpenGLDebugMessage> startupMessages = _logger->loggedMessages();

            connect(_logger, &QOpenGLDebugLogger::messageLogged,
                    this, &OpenGLDebugLogger::onMessageLogged, Qt::DirectConnection);

            if(!startupMessages.isEmpty())
            {
                for(const auto& startupMessage : startupMessages)
                    onMessageLogged(startupMessage);
            }

            _logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
            _logger->enableMessages();
        }
        else
        {
            qDebug() << "Debugging requested but logger failed to initialize";

            const auto* context = QOpenGLContext::currentContext();
            Q_ASSERT(context != nullptr);

            if(!context->hasExtension(QByteArrayLiteral("GL_KHR_debug")))
                qDebug() << "...GL_KHR_debug not available";
        }
    }
}

OpenGLDebugLogger::~OpenGLDebugLogger()
{
    if(_logger != nullptr && _logger->isLogging())
    {
        _logger->disableMessages();
        _logger->stopLogging();
    }
}

void OpenGLDebugLogger::onMessageLogged(const QOpenGLDebugMessage& message) const
{
    if((message.severity() & _debugLevel) == 0)
        return;

    qDebug() << "OpenGL:" << message.message();
}
