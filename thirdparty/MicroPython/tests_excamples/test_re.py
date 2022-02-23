#test_re.py
import ure as re

r = re.compile(".+")
m = r.match("abc")
print(m.group(0))#abc
str(r)#'<re 2000b5c0>',正则表达式编译后所在的内存地址
str(m)#'<match num=1>'
r = re.compile("(.+)1")
m = r.match("xyz781")
print(m.group(0))#xyz781
print(m.group(1))#xyz78
r = re.compile("[a-cu-z]")
m = r.match("a")
print(m.group(0))#a
m = r.match("z")
print(m.group(0))#z
m = r.match("d")
print(m)#None
m = r.match("A")
print(m)#None

def multiply(m):
    return str(int(m.group(0)) * 2)

print(re.sub("\d+", multiply, "10 20 30 40 50"))#20 40 60 80 100

def A():
    return "A"

print(re.sub("a", A(), "aBCBABCDabcda."))#ABCBABCDAbcdA.

m = re.search(r"[a\-z]", "-")
print(m.group(0))#-
m = re.search(r"[a\]z]", "a")
print(m.group(0))#a

ure.compile("^a|b[0-9]\w$", ure.DEBUG)#显示已编译的正则表达式调试信息

def print_spans(match):
    print("----")
    try:
        i = 0
        while True:
            print(match.span(i), match.start(i), match.end(i))
            i += 1
    except IndexError:
        pass

m = re.match(r"(([0-9]*)([a-z]*)[0-9]*)", "1234hello567")#匹配1234hello567中的数字串和字母串
print_spans(m)
'''
----
(0, 12) 0 12
(0, 12) 0 12
(0, 4) 0 4
(4, 9) 4 9
'''
print(m.group(0))#1234hello567
print(m.group(1))#1234hello567
print(m.group(2))#1234
print(m.group(3))#hello
