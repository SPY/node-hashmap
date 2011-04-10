var hashmap = require('../hashmap');

var hash = new hashmap.HashMap();

var a = function(){return 12;},//{ b : 12},
    c = function(){return 45;},
    d = {};

var l = function(){console.log.apply(console, arguments)}
l('empty', hash.empty())
hash.set('ololo', 55)
hash.set(a, 12);
hash.set(c, 8);
hash.set(4, 5);
hash.set(6, 7);
l('ololo', hash.get('ololo'))
l(4,hash.get(4))
l(6,hash.get(6))
l('a ', hash.get(a));
l('c ', hash.get(c));
l('fn ', hash.each(function(k,v) {l(k,v);this.a = 8; throw 'ololo';}, d))
l('d ', require('util').inspect(d));
l('remove a : ', hash.remove(a))
hash.remove('adb');
l('a ', hash.get(a))
l('empty', hash.empty())
l('clear')
hash.clear()
l('empty', hash.empty())