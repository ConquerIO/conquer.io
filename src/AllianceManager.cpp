#include "AllianceManager.hpp"
#include <Entity.hpp>

#include <algorithm>

namespace{
    constexpr float REQUEST_TIMEOUT = 15.0f;
    constexpr float REJECT_COOLDOWN = 30.0f;
}

void AllianceManager::reset(std::size_t entityCount){
    n = entityCount;
    const std::size_t size = n * n;

    allied.assign(size, 0);
    cooldownUntil.assign(size, 0.0f);
    requests.assign(size, RequestSlot{});
}

std::size_t AllianceManager::index(const Entity* a, const Entity* b) const{
    return a->getId() * n + b->getId();
}

bool AllianceManager::valid(const Entity* entity) const{
    return entity != nullptr && entity->getId() < n;
}

bool AllianceManager::areAllied(const Entity* a, const Entity* b) const{
    if (!valid(a) || !valid(b) || a == b) {
        return false;
    }
    return allied[index(a, b)] != 0;
}

bool AllianceManager::hasPendingFrom(const Entity* from, const Entity* to) const{
    if (!valid(from) || !valid(to) || from == to) {
        return false;
    }
    return requests[index(from, to)].active;
}

bool AllianceManager::hasPendingTo(const Entity* to, const Entity* from) const{
    return hasPendingFrom(from, to);
}

float AllianceManager::cooldownRemaining(const Entity* a, const Entity* b, float now) const{
    if (!valid(a) || !valid(b) || a == b) {
        return 0.0f;
    }
    return std::max(0.0f, cooldownUntil[index(a, b)] - now);
}

bool AllianceManager::canRequest(const Entity* from, const Entity* to, float now) const{
    if (!valid(from) || !valid(to) || from == to) {
        return false;
    }
    if (areAllied(from, to)) {
        return false;
    }
    if (cooldownRemaining(from, to, now) > 0.0f) {
        return false;
    }
    if (requests[index(from, to)].active) {
        return false;
    }

    return true;
}

bool AllianceManager::sendRequest(Entity* from, Entity* to, float now, float respondAt){
    if (!canRequest(from, to, now)) return false;

    //si ya te ha enviado el otro una solicitud antes, vamos a hacer como que si se la envias es como aceptarla
    if (requests[index(to, from)].active){
        return accept(from, to, now);
    }

    RequestSlot& slot = requests[index(from, to)];
    slot.active = true;
    slot.from = from;
    slot.to = to;
    slot.createdAt = now;
    slot.expiresAt = now + REQUEST_TIMEOUT;
    slot.respondAt = respondAt;

    return true;
}

bool AllianceManager::accept(Entity* receiver, Entity* requester, float now){
    (void)now;

    if (!valid(receiver) || !valid(requester) || receiver == requester){
        return false;
    }

    RequestSlot& request = requests[index(requester, receiver)];
    if (!request.active) {
        return false;
    }

    allied[index(receiver, requester)] = 1;
    allied[index(requester, receiver)] = 1;

    clearRequest(index(receiver, requester));
    clearRequest(index(requester, receiver));

    cooldownUntil[index(receiver, requester)] = 0.0f;
    cooldownUntil[index(requester, receiver)] = 0.0f;

    return true;
}

void AllianceManager::reject(Entity* receiver, Entity* requester, float now){
    if (!valid(receiver) || !valid(requester) || receiver == requester){
        return;
    }
    clearRequest(index(receiver, requester));
    clearRequest(index(requester, receiver));

    setCooldown(receiver, requester, now + REJECT_COOLDOWN);
}

void AllianceManager::breakAlliance(Entity* a, Entity* b){
    if (!valid(a) || !valid(b) || a == b) {
        return;
    }

    allied[index(a, b)] = 0;
    allied[index(b, a)] = 0;

    clearRequest(index(a, b));
    clearRequest(index(b, a));
}

void AllianceManager::update(float now){
    for (RequestSlot& slot : requests){
        if (!slot.active) {
            continue;
        }
        if (now < slot.expiresAt) {
            continue;
        }

        Entity* from = slot.from;
        Entity* to = slot.to;

        slot = RequestSlot{};

        if (from != nullptr && to != nullptr){
            setCooldown(from, to, now + REJECT_COOLDOWN);
        }
    }
}

void AllianceManager::removeEntity(Entity* entity){
    if (!valid(entity)) {
        return;
    }

    const std::size_t id = entity->getId();

    for (std::size_t i = 0; i < n; ++i){
        const std::size_t a = id * n + i;
        const std::size_t b = i * n + id;

        allied[a] = 0;
        allied[b] = 0;

        cooldownUntil[a] = 0.0f;
        cooldownUntil[b] = 0.0f;

        requests[a] = RequestSlot{};
        requests[b] = RequestSlot{};
    }
}

std::size_t AllianceManager::countAllies(const Entity* entity) const{
    if (!valid(entity)) {
        return 0;
    }

    const std::size_t id = entity->getId();
    std::size_t count = 0;

    for (std::size_t i = 0; i < n; ++i){
        if (allied[id * n + i] != 0) {
            ++count;
        }
    }

    return count;
}

std::size_t AllianceManager::countPendingTo(const Entity* target) const{
    if (!valid(target)) {
        return 0;
    }

    const std::size_t targetId = target->getId();
    std::size_t count = 0;

    for (std::size_t fromId = 0; fromId < n; ++fromId){
        if (requests[fromId * n + targetId].active) {
            ++count;
        }
    }

    return count;
}

std::size_t AllianceManager::countPendingFrom(const Entity* from) const{
    if (!valid(from)) {
        return 0;
    }

    const std::size_t fromId = from->getId();
    std::size_t count = 0;

    for (std::size_t toId = 0; toId < n; ++toId){
        if (requests[fromId * n + toId].active) {
            ++count;
        }
    }

    return count;
}

bool AllianceManager::firstPendingTo(const Entity* target, Entity*& outFrom,  float& outExpiresAt) const{
    if (!valid(target)) {
        return false;
    }

    const std::size_t targetId = target->getId();
    const RequestSlot* best = nullptr;

    for (std::size_t fromId = 0; fromId < n; ++fromId){
        const RequestSlot& slot = requests[fromId * n + targetId];

        if (!slot.active) {
            continue;
        }

        if (best == nullptr || slot.createdAt < best->createdAt){
            best = &slot;
        }
    }

    if (best == nullptr) {
        return false;
    }

    outFrom = best->from;
    outExpiresAt = best->expiresAt;
    return true;
}

void AllianceManager::clearRequest(std::size_t i){
    requests[i] = RequestSlot{};
}

void AllianceManager::setCooldown(Entity* a, Entity* b, float until){
    if (!valid(a) || !valid(b)){
        return;
    }
    cooldownUntil[index(a, b)] =std::max(cooldownUntil[index(a, b)], until);
    cooldownUntil[index(b, a)] =std::max(cooldownUntil[index(b, a)], until);
}