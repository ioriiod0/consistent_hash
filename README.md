# 一致性哈希的C++实现

一致性哈希是分布式计算领域被广泛应用的一个算法。在许多分布式系统包括 Amazon Dynamo, memcached, Riak 等中都有使用。
一致性哈希的原理比较简单，网上有很多介绍的比较好的文章，也有一些相关的代码，但是这些代码都不太令人满意，因此自己实现了一个。代码很简单，放在了 [github](https://github.com/ioriiod0/consistent_hash) 上面。


##consistent_hash_map

一致性哈希的功能被封装在模板类consistent_hash_map中：

    template <typename T,
            typename Hash,
            typename Alloc = std::allocator<std::pair<const typename Hash::result_type,T > > >
    class consistent_hash_map


consistent_hash_map 使用了stl map风格的接口。实际上其内部也使用了std::map 来管理和维护所有节点。

consistent_hash_map只提供最基本的一致性hash的功能，并不直接支持虚拟节点的概念，但是虚拟节点的概念可以很容易的通过定制的T 和 Hash类型来实现。这样设计的好处在于可以使consitent_hash_map的设计和实现变得非常的简单，同时留给用户以极大的灵活性和可定制性。
后面的例子中将介绍如何实现虚拟节点。


##模板参数

1. T:       consistent hash的节点类型。
2. Hash:    一元函数对象。接收T类型对象作为参数，返回一个整形作为其hash值，该hash值将被用于内部的排序。Hash需在其内部定义result_type 指明返回整形的类型。
3. Alloc:   内存分配器，默认为std::allocator

##member type
    size_type           Hash::reslut_type               hash函数返回值的类型
    value_type          std::pair<const size_type,T>    first为节点的哈希值，second为节点
    iterator            a bidirectional iterator to value_type  双向迭代器
    reverse_iterator    reverse_iterator<iterator>      反向迭代器

##member function
    std::size_t size() const;
    返回consistent_hash_map内的节点数量。

    bool empty() const;
    判断consistent_hash_map是否为空

    std::pair<iterator,bool> insert(const T& node);
    插入一个节点，如果返回值中bool变量为真，iterator则为指向插入节点的迭代器。如果bool为假，表示插入失败，iterator指向已经存在的节点。插入失败因为节点已经存在或者是节点的hash值与其他节点发生冲突。

    void erase(iterator it);
    通过迭代器删除指定节点。

    std::size_t erase(const T& node);
    通过节点值删除指定节点。

    iterator find(size_type hash);
    通过传入的hash值找对其在consistent_hash中对应的节点的迭代器。

    iterator begin();
    iterator end();
    返回对应迭代器

    reverse_iterator rbegin();
    reverse_iterator rend();
    返回对应的反向迭代器

##虚拟节点的例子

整个例子的完整代码[在这](https://github.com/ioriiod0/consistent_hash/blob/master/example.cpp)。
在这个例子中，我们首先定义虚拟节点类型，和其对应的hasher。

    #include <stdint.h>
    #include <boost/format.hpp>
    #include <boost/crc.hpp>

    #include "consistent_hash_map.hpp"

    //所有主机的列表
    const char* nodes[] = {
        "192.168.1.100",
        "192.168.1.101",
        "192.168.1.102",
        "192.168.1.103",
        "192.168.1.104"
    };

    //虚拟节点
    struct vnode_t {
        vnode_t() {}
        vnode_t(std::size_t n,std::size_t v):node_id(n),vnode_id(v) {}

        std::string to_str() const {
            return boost::str(boost::format("%1%-%2%") % nodes[node_id] % vnode_id);
        }
        
        std::size_t node_id; //主机ID,主机在主机列表中的索引
        std::size_t vnode_id; //虚拟节点ID

    };

    //hasher,使用CRC32作为hash算法，注意需要定义result_type
    struct crc32_hasher {
        uint32_t operator()(const vnode_t& node) {
            boost::crc_32_type ret;
            std::string vnode = node.to_str();
            ret.process_bytes(vnode.c_str(),vnode.size());
            return ret.checksum();
        }
        typedef uint32_t result_type;
    };


为每个主机生成100个虚拟节点，然后加入consistent_hash_map中。
    
    typedef consistent_hash_map<vnode_t,crc32_hasher> consistent_hash_t;
    consistent_hash_t consistent_hash_;

    for(std::size_t i=0;i<5;++i) {
        for(std::size_t j=0;j<100;j++) {
            consistent_hash_.insert(vnode_t(i,j));
        }
    }


查找某个hash值对应的vnode 和 主机：

    consistent_hash_t::iterator it;
    it = consistent_hash_.find(290235110);
    //it -> first是该节点的hash值，it -> second是该虚拟节点。
    std::cout<<boost::format("node:%1%,vnode:%2%,hash:%3%") 
                % nodes[it->second.node_id] % it->second.vnode_id % it->first << std::endl;



遍历consistent_hash中的所有的vnode,统计每个虚拟节点的key的数量和每个主机包含key的数量：

    std::size_t sums[] = {0,0,0,0,0};
    consistent_hash_t::iterator i = consistent_hash_.begin(); //第一个节点
    consistent_hash_t::reverse_iterator j = consistent_hash_.rbegin(); //最后一个节点
    std::size_t n = i->first + UINT32_MAX - j->first; //计算第一个节点包含的key的数量
    std::cout<<boost::format("vnode:%1%,hash:%2%,contains:%3%") 
            % i->second.to_str() % i->first % n << std::endl;
    sums[i->second.node_id] += n; //更新主机包含的key数量。

    //计算剩余所有节点包含的key的数量,并更新主机包括的key的数量。
    uint32_t priv = i->first;
    uint32_t cur;
    consistent_hash_t::iterator end = consistent_hash_.end();
    while(++i != end) {
        cur = i->first;
        n = cur - priv;
        std::cout<<boost::format("vnode:%1%,hash:%2%,contains:%3%") 
            % i->second.to_str() % cur % n << std::endl;
        sums[i->second.node_id] += n;
        priv = cur;
    }
        
    for(std::size_t i=0;i<5;++i) {
        std::cout<<boost::format("node:%1% contains:%2%") % nodes[i] % sums[i] <<std::endl; 
    }










