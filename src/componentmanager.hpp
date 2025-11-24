#pragma once

#include "common.hpp"

#include "entity.hpp"

class componentmanager {
  public:
    template<typename T>
    void enroll() noexcept {
      const auto* name = typeid(T).name();

      assert(_types.find(name) == _types.end() && "Registering component type more than once.");

      _types.emplace(name, _next);

      ++_next;
    }

   	template<typename T>
    componenttype get()
    {
      const auto* name = typeid(T).name();

      auto it = _types.find(name);
      assert(it != _types.end() && "Component not registered before use.");

      return it->second;
    }
    
    template<typename T>
  	void add(entity entity, T component)
  	{
  		// GetComponentArray<T>()->InsertData(entity, component);
  	}
   
  	template<typename T>
  	void remove(entity entity)
  	{
  		// GetComponentArray<T>()->RemoveData(entity);
  	}

  private:
    uint64_t _next{0};

    std::unordered_map<std::string, componenttype> _types;
};
