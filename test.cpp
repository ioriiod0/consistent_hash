
#include <iostream>
#include <string>
#include <boost/functional/hash.hpp>

#include <stdint.h>
#include <boost/format.hpp>
#include <boost/crc.hpp>

#include "consistent_hash_map.hpp"

struct crc32_hasher {
    uint32_t operator()(const std::string& node) {
        boost::crc_32_type ret;
        ret.process_bytes(node.c_str(),node.size());
        return ret.checksum();
    }
    typedef uint32_t result_type;
};


int main(int argc, char const *argv[])
{
    typedef consistent_hash_map<std::string,crc32_hasher> consistent_hash_t;
    consistent_hash_t consistent_hash_;
    boost::format node_fmt("192.168.1.%1%");

    for(std::size_t i=0;i<3;++i) {
        std::string node = boost::str(node_fmt % i);
        consistent_hash_.insert(node);
        std::cout<<boost::format("add node: %1%") % node << std::endl;
    }


{
    std::cout<<"========================================================="<<std::endl;
    for(consistent_hash_t::iterator it = consistent_hash_.begin();it != consistent_hash_.end(); ++it) {
        std::cout<<boost::format("node:%1%,%2%") % it->second % it->first << std::endl;
    }
}


{
    consistent_hash_t::iterator it;
    it = consistent_hash_.find(290235110);
    std::cout<<boost::format("node:%1%,%2%") % it->second % it->first << std::endl;
}

{
    consistent_hash_t::iterator it;
    it = consistent_hash_.find(2286285664);
    std::cout<<boost::format("node:%1%,%2%") % it->second % it->first << std::endl;
}

{
    consistent_hash_t::iterator it;
    it = consistent_hash_.find(4282565578);
    std::cout<<boost::format("node:%1%,%2%") % it->second % it->first << std::endl;
}  


  std::cout<<"========================================================="<<std::endl;
{
    std::string node = boost::str(node_fmt % 1);
    consistent_hash_.erase(node);
    for(consistent_hash_t::iterator it = consistent_hash_.begin();it != consistent_hash_.end(); ++it) {
        std::cout<<boost::format("node:%1%,%2%") % it->second % it->first << std::endl;
    }
}

  std::cout<<"========================================================="<<std::endl;
{
    consistent_hash_t::iterator it;
    it = consistent_hash_.find(4282565578);
    std::cout<<boost::format("node:%1%,%2%") % it->second % it->first << std::endl;
    std::cout<<"-------------------------------------------"<<std::endl;
    consistent_hash_.erase(it);
    for(consistent_hash_t::iterator it = consistent_hash_.begin();it != consistent_hash_.end(); ++it) {
        std::cout<<boost::format("node:%1%,%2%") % it->second % it->first << std::endl;
    }
}  

  std::cout<<"========================================================="<<std::endl;
{
    std::cout<<"-------------------------------------------"<<std::endl;
    consistent_hash_t::iterator it;
    it = consistent_hash_.find(4282565578);
    std::cout<<boost::format("node:%1%,%2%") % it->second % it->first << std::endl;
    std::cout<<"-------------------------------------------"<<std::endl;

    it = consistent_hash_.find(4282565576);
    std::cout<<boost::format("node:%1%,%2%") % it->second % it->first << std::endl;
    std::cout<<"-------------------------------------------"<<std::endl;

    consistent_hash_.erase(it);

    for(consistent_hash_t::iterator it = consistent_hash_.begin();it != consistent_hash_.end(); ++it) {
        std::cout<<boost::format("node:%1%,%2%") % it->second % it->first << std::endl;
    }
    std::cout<<"-------------------------------------------"<<std::endl;
}


  std::cout<<"========================================================="<<std::endl;
{
    std::cout<<"-------------------------------------------"<<std::endl;
    consistent_hash_t::iterator it;
    it = consistent_hash_.find(4282565578);
    if(it == consistent_hash_.end()) {
        std::cout<<"not found, consistent_hash is empty"<<std::endl;
    }
}

    return 0;
}