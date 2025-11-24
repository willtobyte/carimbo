#pragma once

#include "common.hpp"

#include "entity.hpp"

class componentmanager {
  public:
    template<typename T>
    void enroll() noexcept {
      const auto* name = typeid(T).name();

      assert(types.find(name) == types.end() && "Registering component type more than once.");

      types.emplace(name, next);

      ++next;
    }

   	template<typename T>
    componenttype get()
    {
      const auto* name = typeid(T).name();

      auto it = types.find(name);
      assert(it != types.end() && "Component not registered before use.");

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
    uint64_t next{0};

    std::unordered_map<std::string, componenttype> types;
};
