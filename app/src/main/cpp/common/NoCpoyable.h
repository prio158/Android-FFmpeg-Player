//
// Created by chen_zi_rui on 2024/4/14.
//

#ifndef PLAYER_NOCPOYABLE_H
#define PLAYER_NOCPOYABLE_H

/**
 * @brief 如果不想让一个类型被拷贝复制，被拷贝构造，就继承该类
 * */
class NonCopyable {

public:
    NonCopyable() = default;

    ~NonCopyable() = default;

    NonCopyable(const NonCopyable &) = delete;

    NonCopyable &operator=(const NonCopyable &) = delete;
};


#endif //PLAYER_NOCPOYABLE_H
