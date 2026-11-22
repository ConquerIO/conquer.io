#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class Entity;

class AllianceManager{
    public:
        struct RequestSlot{
            bool active = false;
            Entity* from = nullptr;
            Entity* to = nullptr;
            float createdAt = 0.0f; //esta tiene un tiempo para aceptarse
            float expiresAt = 0.0f;
            float respondAt = 0.0f; //este es el que mide la respuesta del bot para si acepta esta la alianza
        };

        void reset(std::size_t entityCount);

        bool areAllied(const Entity* a, const Entity* b) const;
        bool hasPendingFrom(const Entity* from, const Entity* to) const;
        bool hasPendingTo(const Entity* to, const Entity* from) const;
        float cooldownRemaining(const Entity* a, const Entity* b, float now) const;

        bool canRequest(const Entity* from, const Entity* to, float now) const;
        //si es destino es un bot hay que pasar el retardo
        bool sendRequest(Entity* from, Entity* to, float now, float respondAt = 0.0f);

        bool accept(Entity* receiver, Entity* requester, float now);
        void reject(Entity* receiver, Entity* requester, float now);
        void breakAlliance(Entity* a, Entity* b);

        void update(float now);
        void removeEntity(Entity* entity);

        std::size_t countAllies(const Entity* entity) const;
        std::size_t countPendingTo(const Entity* target) const;
        bool firstPendingTo(const Entity* target, Entity*& outFrom,  float& outExpiresAt) const;

        template <typename Func>
        void forEachPending(Func&& func) const
        {
            for (const RequestSlot& slot : requests)
            {
                if (slot.active) {
                    func(slot);
                }
            }
        }

        std::size_t capacity() const { 
            return n; 
        }

    private:
        std::size_t n = 0;
        std::vector<std::uint8_t> allied;  
        std::vector<float> cooldownUntil;   
        std::vector<RequestSlot> requests; 

        std::size_t index(const Entity* a, const Entity* b) const;
        bool valid(const Entity* entity) const;
        void clearRequest(std::size_t i);
        void setCooldown(Entity* a, Entity* b, float until);
};