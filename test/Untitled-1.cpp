下面是稍作修改的代码：




void Lock(atomic_flag* lock) { while (lock.test_and_set()); }
void Unlock(atmoic_flag* lock) { lock.clear(); }

