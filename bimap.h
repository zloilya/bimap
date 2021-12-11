#pragma once

#include <cstddef>
#include <memory>

struct tag_left;
struct tag_right;

template <class Tag> struct node_t {
  node_t *left;
  node_t *right;
  node_t *parent;
  node_t() : left(nullptr), right(nullptr), parent(nullptr) {}

  static node_t *find_next(node_t *cur) {
    if (cur->right != nullptr) {
      node_t *tmp = cur->right;
      while (tmp->left != nullptr) {
        tmp = tmp->left;
      }
      return tmp;
    }
    node_t *tmp = cur->parent;
    while (tmp->right == cur) {
      cur = tmp;
      tmp = tmp->parent;
    }
    return tmp;
  }

  // reject humanity, go to monkey
  static node_t find_prev(node_t *cur) {
    if (cur->left != nullptr) {
      node_t *tmp = cur->left;
      while (tmp->right != nullptr) {
        tmp = tmp->right;
      }
      return tmp; // go to monkey
    }
    node_t *tmp = cur->parent;
    while (tmp->left == cur) {
      cur = tmp;
      tmp = tmp->parent;
    }
    return tmp;
  }

  static node_t *g_parent(node_t *cur) {
    if (cur == nullptr) {
      return nullptr;
    }
    if (cur->parent == nullptr) {
      return nullptr;
    }
    return cur->parent->parent;
  }

  static void rotate_left(node_t *cur) {
    if (cur == nullptr) {
      return;
    }
    node_t *par = cur->parent;
    node_t *right = cur->right;
    if (par != nullptr) {
      if (par->left == cur) {
        par->left = right;
      } else {
        par->right = right;
      }
    }
    node_t *tmp = right->left;
    right->left = cur;
    cur->right = tmp;
    cur->parent = right;
    right->parent = par;
    if (cur->right != nullptr) {
      cur->right->parent = cur;
    }
  }

  static void rotate_right(node_t<Tag> *cur) {
    if (cur == nullptr) {
      return;
    }
    node_t<Tag> *par = cur->parent;
    node_t<Tag> *left = cur->left;
    if (par != nullptr) {
      if (par->left == cur) {
        par->left = left;
      } else {
        par->right = left;
      }
    }
    node_t<Tag> *tmp = left->right;
    left->right = cur;
    cur->left = tmp;
    cur->parent = left;
    left->parent = par;
    if (cur->left != nullptr) {
      cur->left->parent = cur;
    }
  }

  static node_t<Tag> *splay(node_t<Tag> *cur) {
    if (cur == nullptr) {
      return cur;
    }

    while (cur->parent != nullptr && g_parent(cur) != nullptr) {
      if (cur == cur->parent->left) {
        if (g_parent(cur) == nullptr || g_parent(cur)->parent == nullptr) {
          rotate_right(cur->parent);
        } else if (cur->parent == g_parent(cur)->left) {
          rotate_right(g_parent(cur));
          rotate_right(cur->parent);
        } else {
          rotate_right(cur->parent);
          rotate_left(cur->parent);
        }
      } else {
        if (g_parent(cur) == nullptr || g_parent(cur)->parent == nullptr) {
          rotate_left(cur->parent);
        } else if (cur->parent == g_parent(cur)->right) {
          rotate_left(g_parent(cur));
          rotate_left(cur->parent);
        } else {
          rotate_left(cur->parent);
          rotate_right(cur->parent);
        }
      }
    }
    return cur;
  }

  static node_t<Tag> *find_max(node_t<Tag> *cur) {
    if (cur == nullptr) {
      return cur;
    }
    if (cur->right != nullptr) {
      return find_max(cur->right);
    } else {
      return splay(cur);
    }
  }

  static node_t<Tag> *find_min_1(node_t<Tag> *cur) {
    if (cur == nullptr) {
      return splay(cur);
    }
    if (cur->left != nullptr) {
      return find_min_1(cur->left);
    } else {
      return splay(cur);
    }
  }

  static node_t<Tag> *merge(node_t<Tag> *left, node_t<Tag> *right) {
    if (left == nullptr && right == nullptr) {
      return nullptr;
    }
    left = find_max(left);
    if (left != nullptr) {
      left->right = right;
      if (right != nullptr) {
        right->parent = left;
      }
    } else {
      return right;
    }
    return left;
  }

  static void erase_1(node_t<Tag> * fake_node) {
    if (fake_node->left != nullptr) {
      fake_node->left->parent = nullptr;
    }
    node_t<Tag> *tmp_left = fake_node->left->left;
    node_t<Tag> *tmp_right = fake_node->left->right;
    fake_node->left = merge(tmp_left, tmp_right);
    if (fake_node->left != nullptr) {
      fake_node->left->parent = fake_node;
    } else {
      fake_node->left = nullptr;
    }
  }
};

template <typename Left, typename Right>
struct node_val : node_t<tag_left>, node_t<tag_right> {
  using left_t = Left;
  using right_t = Right;
  left_t left;
  right_t right;

  template <class First, class Second>
  explicit node_val(First &&first, Second &&second)
      : left(std::forward<First>(first)), right(std::forward<Second>(second)) {}

  template <class Tag> auto const &get() const {
    if constexpr (std::is_same_v<tag_left, Tag>) {
      return left;
    } else {
      return right;
    }
  }
};

template <class Data, class Node_val, class Tag, class Comp> struct Splay : Comp, node_t<Tag> {

  static Node_val *to_node_val(node_t<Tag> *cur) {
    return static_cast<Node_val *>(cur);
  }

  Splay() = default;

  Splay(Splay const &splay1) : Comp(splay1.comp()), node_t<Tag>() {}

  explicit Splay(Comp const &comp1) : Comp(comp1), node_t<Tag>() {}

  explicit Splay(Comp &&comp1) : Comp(std::move(comp1)), node_t<Tag>() {}

  Comp *comp_p() { return this; }

  const Comp &comp() const { return *this; }


  //node_t<Tag> *fake_node() { return (this); }
  node_t<Tag> *fake_node() const { return (node_t<Tag> *) (this); }

  node_t<Tag> *lower_bound(const Data &data) const {
    return const_cast<Splay *>(this)->lower_bound(data, fake_node()->left, comp());
  }

  node_t<Tag> *upper_bound(const Data &data) const {
    return const_cast<Splay *>(this)->upper_bound(data, fake_node()->left, fake_node(), comp());
  }

  //возвращает найденый элемент или ближайший элемент
  node_t<Tag> *find(node_t<Tag> *cur, Data const &x) const {
    return const_cast<Splay *>(this)->find(cur, x, comp());
  }

  //возвращает найденый элемент или nullptr
  node_t<Tag> *find_null(Data const &x) const {
    return const_cast<Splay *>(this)->find_null(fake_node()->left, x, comp());
  }

  void erase() const { const_cast<Splay *>(this)->erase_1(fake_node()); }

  void add(node_t<Tag> *cur) const { const_cast<Splay *>(this)->add_1(cur, fake_node()); }

  void add_1(node_t<Tag> *cur, node_t<Tag> *root) {
    std::pair<node_t<Tag> *, node_t<Tag> *> p =
        split(root->left, (to_node_val(cur))->template get<Tag>());
    cur->left = p.first;
    cur->right = p.second;
    if (p.first != nullptr)
      cur->left->parent = cur;
    if (p.second != nullptr)
      cur->right->parent = cur;
    cur->parent = root;
    root->left = cur;
  }

  std::pair<node_t<Tag> *, node_t<Tag> *> split(node_t<Tag> *cur, Data const &x) const {
    return const_cast<Splay *>(this)->split(cur, x, comp());
  }

  node_t<Tag> *find_min() const{ return const_cast<Splay *>(this)->find_min_1(fake_node()->left); }

  static node_t<Tag> *lower_bound(const Data &data, node_t<Tag> *root,
                                  Comp const &comp) {
    node_t<Tag> *cur = find(root, data, comp);
    if (comp((to_node_val(cur))->template get<Tag>(), data) ==
        comp(data, (to_node_val(cur))->template get<Tag>())) {
      return cur;
    }
    if (comp(data, (to_node_val(cur))->template get<Tag>())) {
      return cur;
    }
    return node_t<Tag>::find_next(cur);
  }

  static node_t<Tag> *upper_bound(const Data &data, node_t<Tag> *root,
                                  node_t<Tag> *end, Comp const &comp) {
    node_t<Tag> *cur = lower_bound(data, root, comp);
    if (cur == end) {
      return end;
    }
    if (comp((to_node_val(cur))->template get<Tag>(), data) ==
        comp(data, (to_node_val(cur))->template get<Tag>())) {
      return node_t<Tag>::find_next(cur);
    }
    return cur;
  }

  //возвращает найденый элемент или ближайший элемент
  static node_t<Tag> *find(node_t<Tag> *cur, Data const &x, Comp const &comp) {
    if (cur == nullptr) {
      return cur;
    }
    if (comp((to_node_val(cur))->template get<Tag>(), x) ==
        comp(x, (to_node_val(cur))->template get<Tag>())) {
      return node_t<Tag>::splay(cur);
    }
    if (comp((to_node_val(cur))->template get<Tag>(), x)) {
      if (cur->right == nullptr) {
        return node_t<Tag>::splay(cur);
      }
      return find(cur->right, x, comp);
    } else {
      if (cur->left == nullptr) {
        return node_t<Tag>::splay(cur);
      }
      return find(cur->left, x, comp);
    }
  }

  //возвращает найденый элемент или nullptr
  static node_t<Tag> *find_null(node_t<Tag> *cur, Data const &x, Comp const &comp) {
    if (cur == nullptr) {
      return nullptr;
    }
    if (comp((to_node_val(cur))->template get<Tag>(), x) ==
        comp(x, (to_node_val(cur))->template get<Tag>())) {
      return node_t<Tag>::splay(cur);
    }
    if (comp((to_node_val(cur))->template get<Tag>(), x)) {
      if (cur->right == nullptr) {
        node_t<Tag>::splay(cur);
        return nullptr;
      }
      return find_null(cur->right, x, comp);
    } else {
      if (cur->left == nullptr) {
        node_t<Tag>::splay(cur);
        return nullptr;
      }
      return find_null(cur->left, x, comp);
    }
  }

  static std::pair<node_t<Tag> *, node_t<Tag> *>
  split(node_t<Tag> *cur, Data const &x, Comp comp) {
    if (cur == nullptr) {
      return {nullptr, nullptr};
    }
    cur = find(cur, x, comp);
    if (comp(x, (to_node_val(cur))->template get<Tag>())) {
      node_t<Tag> *tmp = cur->left;
      cur->left = nullptr;
      return {tmp, cur};
    }
    node_t<Tag> *right = cur->right;
    cur->right = nullptr;
    return {cur, right};
  }

  void swap(Splay &splay1) {
    std::swap(*fake_node(), *splay1.fake_node());
    if (fake_node()->left != nullptr) {
      fake_node()->left->parent = fake_node();
    }
    if (splay1.fake_node()->left != nullptr) {
      splay1.fake_node()->left->parent = splay1.fake_node();
    }
    std::swap(*comp_p(), *splay1.comp_p());
  }
};

template <typename Left, typename Right, typename CompareLeft = std::less<Left>,
    typename CompareRight = std::less<Right>>
struct bimap :
    Splay<Left, node_val<Left, Right>, tag_left, CompareLeft>,
    Splay<Right, node_val<Left, Right>, tag_right, CompareRight> {
public:
  using left_t = Left;
  using right_t = Right;
  using Splay_left = Splay<Left, node_val<Left, Right>, tag_left, CompareLeft>;
  using Splay_right =
      Splay<Right, node_val<Left, Right>, tag_right, CompareRight>;

private:
  static node_val<Left, Right> *to_node_val(node_t<tag_left> *cur) {
    return static_cast<node_val<Left, Right> *>(cur);
  }

  static node_val<Left, Right> *to_node_val(node_t<tag_right> *cur) {
    return static_cast<node_val<Left, Right> *>(cur);
  }

  Splay_left *splay_left() const { return (Splay_left *) this; }
  // Splay_left const*splay_left() const { return this; }

  Splay_right *splay_right() const { return (Splay_right *) this; }

  std::size_t bi_size;

public:
  template <bool RorL> struct base_iterator {
    using value_type = std::conditional_t<RorL, right_t, left_t>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type *;
    using reference = const value_type &;
    using iterator_category = std::bidirectional_iterator_tag;
    friend bimap;

  protected:
    using tag = std::conditional_t<RorL, tag_right, tag_left>;
    node_t<tag> *cur;

  public:
    explicit base_iterator(node_t<tag> *tmp) : cur(tmp) {}

    // Элемент на который сейчас ссылается итератор.
    // Разыменование итератора end_left() неопределено.
    // Разыменование невалидного итератора неопределено.
    value_type const &operator*() const {
      return (to_node_val(cur))->template get<tag>();
    }

    // Переход к следующему по величине left'у.
    // Инкремент итератора end_left() неопределен.
    // Инкремент невалидного итератора неопределен.
    base_iterator &operator++() {
      cur = node_t<tag>::find_next(cur);
      return *this;
    }
    base_iterator operator++(int) {
      auto tmp = *this;
      operator++();
      return tmp;
    }

    // Переход к предыдущему по величине left'у.
    // Декремент итератора begin_left() неопределен.
    // Декремент невалидного итератора неопределен.
    base_iterator &operator--() {
      cur = node_t<tag>::find_prev(cur);
      return *this;
    }
    base_iterator operator--(int) {
      auto tmp = *this;
      operator--();
      return tmp;
    }

    // left_iterator ссылается на левый элемент некоторой пары.
    // Эта функция возвращает итератор на правый элемент той же пары.
    // end_left().flip() возращает end_right().
    // end_right().flip() возвращает end_left().
    // flip() невалидного итератора неопределен.
    base_iterator<!RorL> flip() const {
      if constexpr (RorL) {
        if (cur->parent == nullptr) {
          return base_iterator<!RorL>(static_cast<bimap *>(cur));
        }
        return base_iterator<!RorL>(to_node_val(cur));
      } else {
        if (cur->parent == nullptr) {
          return base_iterator<!RorL>(static_cast<bimap *>(cur));
        }
        return base_iterator<!RorL>(to_node_val(cur));
      }
    }

    friend bool operator==(const base_iterator &lhs, const base_iterator &rhs) {
      return lhs.cur == rhs.cur;
    }
    friend bool operator!=(const base_iterator &lhs, const base_iterator &rhs) {
      return lhs.cur != rhs.cur;
    }
  };
  using right_iterator = base_iterator<true>;
  using left_iterator = base_iterator<false>;

  // Создает bimap не содержащий ни одной пары.
  bimap(CompareLeft compare_left = CompareLeft(),
        CompareRight compare_right = CompareRight())
      : Splay_left(std::move(compare_left)),
        Splay_right(std::move(compare_right)), bi_size(0) {}

  // Конструкторы от других и присваивания
  bimap(bimap const &other)
      : Splay_left(other), Splay_right(other), bi_size(0) {
    copy_tree(other.splay_left()->fake_node()->left);
  }
  bimap(bimap &&other) noexcept
      : Splay_left(std::move(*other.splay_left())),
        Splay_right(std::move(*other.splay_right())),
        bi_size(std::move(other.bi_size)) {}

  bimap &operator=(bimap const &other) {
    if (&other == this) {
      return *this;
    }
    bimap b = bimap(other);
    swap(b);
    return *this;
  }
  bimap &operator=(bimap &&other) noexcept {
    if (&other == this) {
      return *this;
    }
    swap(other);
    return *this;
  }

  // Деструктор. Вызывается при удалении объектов bimap.
  // Инвалидирует все итераторы ссылающиеся на элементы этого bimap
  // (включая итераторы ссылающиеся на элементы следующие за последними).
  ~bimap() { delete_tree(splay_left()->fake_node()->left); }

  // Вставка пары (left, right), возвращает итератор на left.
  // Если такой left или такой right уже присутствуют в bimap, вставка не
  // производится и возвращается end_left().
private:
  bool find_elem(left_t const &left, right_t const &right) {
    if (splay_left()->fake_node()->left != nullptr) {
      if (splay_left()->find_null(left) != nullptr) {
        return false;
      }
      if (splay_right()->find_null(right) != nullptr) {
        return false;
      }
    }
    return true;
  }

  template <class First, class Second>
  left_iterator insert_common(First &&first, Second &&second) {
    if (!find_elem(std::forward<First>(first), std::forward<Second>(second))) {
      return end_left();
    }
    auto *nw = new node_val<left_t, right_t>(std::forward<First>(first),
                                             std::forward<Second>(second));
    bi_size++;
    splay_left()->add(nw);
    splay_right()->add(nw);
    return left_iterator(nw);
  }

  void swap(bimap &other) {
    std::swap(bi_size, other.bi_size);
    splay_left()->swap(*other.splay_left());
    splay_right()->swap(*other.splay_right());
  }

public:
  left_iterator insert(left_t const &left, right_t const &right) {
    return insert_common(left, right);
  }
  left_iterator insert(left_t &&left, right_t const &right) {
    return insert_common(std::move(left), right);
  }
  left_iterator insert(left_t const &left, right_t &&right) {
    return insert_common(left, std::move(right));
  }
  left_iterator insert(left_t &&left, right_t &&right) {
    return insert_common(std::move(left), std::move(right));
  }

private:
  void erase_common() {
    node_val<left_t, right_t> *tmp =
        to_node_val(splay_left()->fake_node()->left);
    bi_size--;
    splay_left()->erase();
    splay_right()->erase();
    delete tmp;
  }

public:
  // Удаляет элемент и соответствующий ему парный.
  // erase невалидного итератора неопределен.
  // erase(end_left()) и erase(end_right()) неопределены.
  // Пусть it ссылается на некоторый элемент e.
  // erase инвалидирует все итераторы ссылающиеся на e и на элемент парный к e.
  left_iterator erase_left(left_iterator it) {
    splay_right()->splay(to_node_val(splay_left()->splay(it.cur)));
    it++;
    erase_common();
    return it;
  }

  // Аналогично erase, но по ключу, удаляет элемент если он присутствует, иначе
  // не делает ничего Возвращает была ли пара удалена
  bool erase_left(left_t const &left) {
    node_t<tag_left> *cur = splay_left()->find_null(left);
    if (cur == nullptr) {
      return false;
    }
    splay_right()->splay(to_node_val(cur));
    erase_common();
    return true;
  }

  right_iterator erase_right(right_iterator it) {
    splay_left()->splay(to_node_val(splay_right()->splay(it.cur)));
    it++;
    erase_common();
    return it;
  }
  bool erase_right(right_t const &right) {
    node_t<tag_right> *cur = splay_right()->find_null(right);
    if (cur == nullptr) {
      return false;
    }
    splay_left()->splay(to_node_val(cur));
    erase_common();
    return true;
  }

  // erase от ренжа, удаляет [first, last), возвращает итератор на последний
  // элемент за удаленной последовательностью
  left_iterator erase_left(left_iterator first, left_iterator last) {
    while (first != last) {
      first = erase_left(first);
    }
    return last;
  }
  right_iterator erase_right(right_iterator first, right_iterator last) {
    while (first != last) {
      first = erase_right(first);
    }
    return last;
  }

  // Возвращает итератор по элементу. Если не найден - соответствующий end()
  left_iterator find_left(left_t const &left) const {
    node_t<tag_left> *cur = splay_left()->find_null(left);
    if (cur == nullptr) {
      return end_left();
    }
    return left_iterator(cur);
  }
  right_iterator find_right(right_t const &right) const {
    node_t<tag_right> *cur = splay_right()->find_null(right);
    if (cur == nullptr) {
      return end_right();
    }
    return right_iterator(cur);
  }

  // Возвращает противоположный элемент по элементу
  // Если элемента не существует -- бросает std::out_of_range
  right_t const &at_left(left_t const &left) const {
    node_t<tag_left> *cur = splay_left()->find_null(left);
    if (cur != nullptr) {
      return (to_node_val(cur))->right;
    }
    throw std::out_of_range("no valid at_left key find");
  }
  left_t const &at_right(right_t const &right) const {
    node_t<tag_right> *cur = splay_right()->find_null(right);
    if (cur != nullptr) {
      return (to_node_val(cur))->left;
    }
    throw std::out_of_range("no valid at_right key find");
  }

  // Возвращает противоположный элемент по элементу
  // Если элемента не существует, добавляет его в bimap и на противоположную
  // сторону кладет дефолтный элемент, ссылку на который и возвращает
  // Если дефолтный элемент уже лежит в противоположной паре - должен поменять
  // соответствующий ему элемент на запрашиваемый (смотри тесты)
  template <class Dummy = void,
            class = std::enable_if_t<
                std::is_default_constructible<right_t>::value, Dummy>>
  right_t const &at_left_or_default(left_t const &left) {
    node_t<tag_left> *cur = splay_left()->find_null(left);
    if (cur != nullptr) {
      return (to_node_val(cur))->right;
    }
    const right_t right = right_t();
    node_t<tag_right> *tmp = splay_right()->find_null(right);
    if (tmp != nullptr) {
      (to_node_val(tmp))->left = left;
      return (to_node_val(tmp))->right;
    }
    return (*insert(left, right).flip());
  }
  template <class Dummy = void,
            class = std::enable_if_t<
                std::is_default_constructible<left_t>::value, Dummy>>
  left_t const &at_right_or_default(right_t const &right) {
    node_t<tag_right> *cur = splay_right()->find_null(right);
    if (cur != nullptr) {
      return (to_node_val(cur))->left;
    }
    const left_t left = left_t();
    node_t<tag_left> *tmp = splay_left()->find_null(left);
    if (tmp != nullptr) {
      (to_node_val(tmp))->right = right;
      return (to_node_val(tmp))->left;
    }
    return (*insert(left, right));
  }

  // lower и upper bound'ы по каждой стороне
  // Возвращают итераторы на соответствующие элементы
  // Смотри std::lower_bound, std::upper_bound.
  left_iterator lower_bound_left(const left_t &left) const {
    return left_iterator(splay_left()->lower_bound(left));
  }
  left_iterator upper_bound_left(const left_t &left) const {
    return left_iterator(splay_left()->upper_bound(left));
  }

  right_iterator lower_bound_right(const right_t &right) const {
    return right_iterator(splay_right()->lower_bound(right));
  }
  right_iterator upper_bound_right(const right_t &right) const {
    return right_iterator(splay_right()->upper_bound(right));
  }

  // Возващает итератор на минимальный по порядку left.
  left_iterator begin_left() const {
    return left_iterator(splay_left()->find_min());
  }
  // Возващает итератор на следующий за последним по порядку left.
  left_iterator end_left() const {
    return left_iterator(splay_left()->fake_node());
  }

  // Возващает итератор на минимальный по порядку right.
  right_iterator begin_right() const {
    return right_iterator(splay_right()->find_min());
  }
  // Возващает итератор на следующий за последним по порядку right.
  right_iterator end_right() const {
    return right_iterator(splay_right()->fake_node());
  }

  // Проверка на пустоту
  bool empty() const { return bi_size == 0; }

  // Возвращает размер бимапы (кол-во пар)
  std::size_t size() const { return bi_size; }

  // операторы сравнения
  friend bool operator==(bimap const &a, bimap const &b) {
    auto iter = b.begin_left();
    for (auto it = a.begin_left(); it != a.end_left(); ++it) {
      if (iter == b.end_left()) {
        return false;
      }
      if ((*it) != (*iter) || (*it.flip()) != (*iter.flip())) {
        return false;
      }
      iter++;
    }
    if (iter != b.end_left()) {
      return false;
    }
    return true;
  }
  friend bool operator!=(bimap const &a, bimap const &b) { return !(a == b); }

private:
  void delete_tree(node_t<tag_left> *cur) {
    if (cur == nullptr) {
      return;
    }
    delete_tree(cur->left);
    delete_tree(cur->right);
    delete (to_node_val(cur));
  }

  void copy_tree(node_t<tag_left> *cur) {
    if (cur == nullptr) {
      return;
    }
    copy_tree(cur->left);
    copy_tree(cur->right);
    insert((to_node_val(cur))->left, (to_node_val(cur))->right);
  }
};
