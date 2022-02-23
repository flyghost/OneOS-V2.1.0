#test_thread.py
import _thread

def func():
    raise ValueError

def thread_entry():
    try:
        func()
    except ValueError:
        print("test pass")
    with lock:
        global n_finished
        n_finished += 1

lock = _thread.allocate_lock()
n_thread = 4
n_finished = 0
for i in range(n_thread):
    _thread.start_new_thread(thread_entry, ())
print(n_thread, n_finished)