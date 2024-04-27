//
// Created by chen_zi_rui on 2024/4/14.
//

#include "Mutex.h"
#include <stdexcept>

Semaphore::Semaphore(uint32_t count) {
    if (sem_init(&m_semaphore, 0, count)) {
        throw std::logic_error("sem init error");
    }
}

Semaphore::~Semaphore() {
    sem_close(&m_semaphore);
}

void Semaphore::wait() {
    //sem_wait 会将m_semaphore--,当m_semaphore==0时候，就会阻塞等待
    if (sem_wait(&m_semaphore) < 0) {
        throw std::logic_error("sem wait error");
    }
}

void Semaphore::notify() {
    // sem_post 会将m_semaphore++
    if (sem_post(&m_semaphore) < 0) {
        throw std::logic_error("sem post error");
    }
}






