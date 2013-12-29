
#include <stdint.h>
#include <iostream>
#include <string>
#include <boost/functional/hash.hpp>
#include <boost/format.hpp>
#include <boost/crc.hpp>

#include "consistent_hash_map.hpp"



const char* nodes[] = {
    "192.168.1.100",
    "192.168.1.101",
    "192.168.1.102",
    "192.168.1.103",
    "192.168.1.104"
};

struct vnode_t {
    vnode_t() {}
    vnode_t(std::size_t n,std::size_t v):node_id(n),vnode_id(v) {}

    std::string to_str() const {
        return boost::str(boost::format("%1%-%2%") % nodes[node_id] % vnode_id);
    }

    std::size_t node_id;
    std::size_t vnode_id;

};


struct crc32_hasher {
    uint32_t operator()(const vnode_t& node) {
        boost::crc_32_type ret;
        std::string vnode = node.to_str();
        std::cout<<"vnode:"<<vnode<<std::endl;
        ret.process_bytes(vnode.c_str(),vnode.size());
        return ret.checksum();
    }
    typedef uint32_t result_type;
};


int main(int argc, char const *argv[])
{
    typedef consistent_hash_map<vnode_t,crc32_hasher> consistent_hash_t;
    consistent_hash_t consistent_hash_;

    for(std::size_t i=0;i<5;++i) {
        for(std::size_t j=0;j<100;j++) {
            consistent_hash_.insert(vnode_t(i,j));
        }
    }


{
    std::cout<<"========================================================="<<std::endl;
    std::size_t sums[] = {0,0,0,0,0};
    consistent_hash_t::iterator i = consistent_hash_.begin();
    consistent_hash_t::reverse_iterator j = consistent_hash_.rbegin();
    std::size_t n = i->first + UINT32_MAX - j->first;
    std::cout<<boost::format("vnode:%1%,hash:%2%,contains:%3%") 
            % i->second.to_str() % i->first % n << std::endl;
    sums[i->second.node_id] += n;

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
    
}


{
    consistent_hash_t::iterator it;
    it = consistent_hash_.find(290235110);
    std::cout<<boost::format("node:%1%,vnode:%2%,hash:%3%") 
                % nodes[it->second.node_id] % it->second.vnode_id % it->first << std::endl;
}

 
    return 0;
}