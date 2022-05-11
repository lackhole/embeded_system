//
// Created by YongGyu Lee on 2022/05/11.
//

#ifndef EMBED_DRAWABLE_MACRO_H_
#define EMBED_DRAWABLE_MACRO_H_

#define EMBED_DRAWABLE_GETTER_SETTER(type, name) \
  const type& name() const { return name##_; }          \
  type& name() { return name##_; }                      \
  auto& name(const type& v) { name##_ = v; return *this; }

#define EMBED_DRAWABLE_PROP_2(type, name)  \
  type name##_; EMBED_DRAWABLE_GETTER_SETTER(type, name)

#define EMBED_DRAWABLE_PROP_3(type, name, value) \
  type name##_ = value; EMBED_DRAWABLE_GETTER_SETTER(type, name)

#define EMBED_DRAWABLE_PROP_N(_1,_2,_3,N,...) EMBED_DRAWABLE_PROP_##N

#define EMBED_DRAWABLE_PROP(...) EMBED_DRAWABLE_PROP_N(__VA_ARGS__,3,2,1)(__VA_ARGS__)

#endif // EMBED_DRAWABLE_MACRO_H_
