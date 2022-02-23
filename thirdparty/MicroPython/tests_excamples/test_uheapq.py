#test_uheapq.py
import uheapq as heapq

#不能为空
try:
    heapq.heappop([])
except IndexError:
    print("IndexError")

#必须为列表类型
try:
    heapq.heappush((), 1)
except TypeError:
    print("TypeError")

#从小到大pop
def pop_and_print(h):
    l = []
    while h:
        l.append(str(heapq.heappop(h)))
    print(" ".join(l))


h = []
heapq.heappush(h, 3)
heapq.heappush(h, 1)
heapq.heappush(h, 2)
print(h)#[1, 3, 2]
pop_and_print(h)#1 2 3

h = [4, 3, 8, 9, 10, 2, 7, 11, 5]
heapq.heapify(h)
print(h)#[2, 3, 4, 5, 10, 8, 7, 11, 9]

heapq.heappush(h, 1)
heapq.heappush(h, 6)
heapq.heappush(h, 12)
print(h)#[1, 2, 4, 5, 3, 8, 7, 11, 9, 10, 6, 12]
pop_and_print(h)#1 2 3 4 5 6 7 8 9 10 11 12
