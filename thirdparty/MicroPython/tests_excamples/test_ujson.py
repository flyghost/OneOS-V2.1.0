#test_ujson.py
import ujson as json

def my_print(o):
    if isinstance(o, dict):
        print("sorted dict", sorted(o.items()))
    else:
        print(o)

print(json.dumps(1.2))#1.2
print(json.dumps({1.5: "hi"}))#{"1.5": "hi"}
print(json.dumps(OrderedDict(((1, 2), (3, 4)))))#{"1": 2, "3": 4}
my_print(json.loads('{"a":null, "b":false, "c":true}'))#sorted dict [('a', None), ('b', False), ('c', True)]
my_print(json.loads('{"a":[], "b":[1], "c":{"3":4}}'))#sorted dict [('a', []), ('b', [1]), ('c', {'3': 4})]
my_print(json.loads('"abc\\ndef"'))#abc
                                   #def
my_print(json.loads('{\n\t"a":[]\r\n, "b":[1], "c":{"3":4}     \n\r\t\r\r\r\n}'))#sorted dict [('a', []), ('b', [1]), ('c', {'3': 4})]

