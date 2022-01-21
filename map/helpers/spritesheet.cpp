#include "spritesheet.h"

#include <QDebug>

Spritesheet::Spritesheet(const QString &filename, int frameWidth, int frameHeight, int fps, QGraphicsItem *parent)
{
    m_parent = parent;
    loadFromFile(filename, frameWidth, frameHeight);
    prepareAnimation(fps);
}

const QImage &Spritesheet::currentFrame() const
{
    return m_frames->at(m_currentFrameIndex);
}

void Spritesheet::loadFromFile(const QString &filename, int frameWidth, int frameHeight)
{
    m_atlas = QImage(filename);
    m_frameWidth = frameWidth;
    m_frameHeight = frameHeight;
    m_currentFrameIndex = 0;

    m_frames = new QList<QImage>();
    for (int h = 0; h < m_atlas.height(); h += frameHeight)
        for (int w = 0; w < m_atlas.height(); w += frameWidth)
            m_frames->append(m_atlas.copy(w, h, frameWidth, frameHeight));
}

void Spritesheet::prepareAnimation(int fps)
{
    m_fps = fps;

    connect(&m_animationTimer, SIGNAL(timeout()), this, SLOT(onAnimationTick()));
    m_animationTimer.start(1000/m_fps);
}

void Spritesheet::onAnimationTick()
{
    if(m_frames->isEmpty())
        return;

    qDebug() << m_currentFrameIndex << " out of " << m_frames->size() << "." << m_frames->at(m_currentFrameIndex).size();

    // Just select next frame in a list of subimages.
    // When there no more next images in a list, start from the very beginning.
    if (forward)
    {
        ++m_currentFrameIndex;
        if (m_currentFrameIndex == m_frames->size() - 1)
            forward = false;
    }
    else
    {
        --m_currentFrameIndex;
        if (m_currentFrameIndex == 0)
            forward = true;
    }


    if (m_parent)
        m_parent->update();
}
