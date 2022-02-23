#test_ucollections.py
from ucollections import deque

deque([1, 2, 3], 10) #不支持初始序列
deque([], 10) #不支持空列表，只支持空元组
deque(()) #只支持固定大小的队列，length必须要输入

d = deque((), 2, True)
d.append(1)
print(d.popleft())#1
d.append(2)
d.append(3)
print(len(d))#2
print(d.popleft(), d.popleft())#2 3

from ucollections import OrderedDict
d = OrderedDict([(10, 20), ("b", 100), (1, 2)])
print(len(d)) #3
print(list(d.keys()))#[10, 'b', 1]
print(list(d.values()))#[20, 100, 2]

del d["b"]
print(len(d))#2
print(list(d.keys()))#[10, 1]
print(list(d.values()))#[20, 2]
print(d[10], d[1])#20 2

d["abc"] = 123
print(len(d))#3
print(list(d.keys()))#[10, 1, 'abc']
print(list(d.values()))#[20, 2, 123]

print(d.popitem())#(10, 20)
print(len(d))#2
print(list(d.keys()))#[1, 'abc']
print(list(d.values()))#[2, 123]

from ucollections import namedtuple

MyTuple = namedtuple("MyTuple", ("id", "name"))
t1 = MyTuple(1, "foo")
t2 = MyTuple(2, "bar")
print(t1.name) #foo
print(t2.name) #bar
print(t1.id, t2.id)#1 2
assert t2.name == t2[1]
print(t2[0], t2[1])#2 bar




