var HashMap = require('../hashmap').HashMap,
    util = require('util'),
    assert = require('assert');

var fn = function(){return 0;},
    obj = { prop : 'val'},
    num = 123,
    str = 'foobar'; 
    
var hash = new HashMap();

console.log('set/remove')
assert.equal(hash.length, 0);
hash.set(str, num);
assert.strictEqual(hash.get(str), num)
assert.equal(hash.length, 1)
assert.strictEqual(hash.remove(str), num);
assert.equal(hash.length, 0);

console.log('replace')
hash.set(str, fn)
assert.strictEqual(hash.get(str), fn)
hash.set(str, obj)
assert.strictEqual(hash.get(str), obj)
assert.equal(hash.length, 1);
assert.equal(hash.hasKey(str), true)
assert.equal(hash.hasKey(obj), false)

console.log('undepecable')
var otherHash = new HashMap();
assert.strictEqual(otherHash.get(str), undefined);
assert.equal(otherHash.length, 0);
otherHash.set(fn, obj).set(obj, fn);
assert.equal(otherHash.length, 2);
assert.strictEqual(otherHash.remove(fn), obj);

console.log('clear')
otherHash.clear()
assert.equal(otherHash.length, 0);
assert.equal(hash.length, 1);

hash.set(num, obj);
hash.set(fn, str);
hash.set(obj, fn);

console.log('iteration')
var scope = {counter : 0}
hash.each(function(key, value, h){
    console.log(util.inspect(key), ' = ', util.inspect(value), ' hash : ', h);
    this.counter++;
}, scope);

assert.equal(hash.length, 4)
assert.equal(scope.counter, hash.length)