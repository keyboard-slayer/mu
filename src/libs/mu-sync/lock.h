#pragma once

typedef int Spinlock;

void spinlock_acquire(Spinlock *self);

void spinlock_release(Spinlock *self);