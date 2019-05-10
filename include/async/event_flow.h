/*
 * The MIT License
 *
 * Copyright 2017 David Curtis.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef ASYNC_EVENT_FLOW_H
#define ASYNC_EVENT_FLOW_H

#include <cstdint>
#include <functional>
#include <async/error.h>
#include <util/traits.h>
#include <unordered_map>
#include <unordered_set>


namespace async
{
namespace event_flow
{

/*

Some important concepts to note:

The terms event, emitter, receiver, etc. usually denote elements in systems
that permit one-to-many or many-to-many connectivity and message (event) flow.
That is not the case here (yet). The initial intended use is to implement flexible
protocol stacks, where each element is connected to at most one corresponding element
above and below it in the stack.

Events

An event type comprises a discriminant type (typically an enum),
a discriminant value, and a signature (a list of parameter types).

An example:

	enum class actions
	{
		start,
		stop,
		kill
	};

	using kill_action = event<actions, actions::kill, std::string const& message>;
	using start_action = event<actions, actions::start, bool flag>;
	using stop_action = event<actions, actions::stop, int number>;

Sources and Sinks

Types that emit and receive events are called sources and sinks, respectively.


A set of related event types share a discriminant type,
and the specific event types in that set correspond to 
values of that discriminant type.

	template<class T, T Value, class... Args>
	class event
	{
	public:
		using sink_f   = std::function<void(Args...)>;
		using source_f = std::function<void(sink_f)>;
	};

T is the type of the discriminant
Value is the value (of type T) that distinguishes the event type.
Args is a parameter list corresponding to the data that
make up the event state. 

Example:

	enum class actions
	{
		start,
		stop,
		kill
	};

	using kill_action  = event<actions, actions::kill, std::string const& message>;
	using start_action = event<actions, actions::start, int n>;
	using stop_action  = event<actions, actions::stop, bool flag>;

	class killer : public emitter<kill_action>
	{};

	class victim : public listener<kill_action, victim> // a bit of CRTP going on here
	{
	public:

		void
		on(kill_action, std::string const& msg)
		{
			std::cout << "got killed: " << msg << std::endl;
		}
	};

	// to emit the event:

	killer oswald;
	victim jfk;

	oswald.fit(jfk);
	oswald.emit<kill_action>("magic bullet");

	// generates output "got killed: magic bullet"

Types that emit and receive events are called sources and sinks, respectively.
Either may be referred to as a fitting (i.e., a thing that connects, as in plumbing fittings).

sink_f and source_f are types of callable objects (functions, functors, lambdas)
that provide type erasure for the mechanisms that connect sinks to sources.

THe mechanics of connection are simple--a source's callable object (source_f)
is invoked, passing the sink's callable object (sink_f) as the argument.
For simple, singular source-sink relationships, this operation is encapsulated 
in the fit() member function on a source:

	class killer : public emitter<kill_action>
	{};

	class victim : public listener<kill_action, victim>
	{
	public:
		// blah blah
	};

	killer oswald;
	victim jfk;

	oswald.fit(jfk);

	// any kill_action events emitted by oswald will be received by jfk

	oswald.emit<kill_action>("magic bullet"); // note the definition of kill_action above

 Connectors and Connectables

A single class may be a source and/or sink for multiple event types. Such a class
is considered a connectable. Specifically, a connectable class is derived from
the connectable<> class template.

A connector type is a carrier for a set of fittings (i.e., sources and/or sinks).
Consider this example:

	enum class noise
	{
		beep,
		ring,
		squeal,
		honk
	};

	using beep_event   = event<noise, noise::beep, int volume>;
	using ring_event   = event<noise, noise::ring, int volume>;
	using squeal_event = event<noise, noise::squeal, int volume>;
	using honk_event   = event<noise, noise::honk, int volume>;

	using annoyance = connector<source<beep_event>, source<ring_event>>;
	using annoyed = connector<sink<beep_event>, sink<ring_event>>;

A source and sink of the same event type are considered complementary. Two connector types A and B
are considered complementary if each element B's parameter list is the complement of the corresponding
element in A's parameter list. Note that annoyance and annoyed (above) are complementary. Complementary
connectors (and their corresponding connectables) may be mated (i.e., connected to allow events to flow
between them).

It is not necessary to explicity define both complementary types. the complement<> helper template
can do this for you:

	using annoyed = complement<annoyance>::type;

The connector types (annoyance and annoyed) serve two purposes:

1) They carry a set of fitting types that are used in the definitions
of connectable classes, i.e., application classes that emit or receive events of the specified types.


	class phone : public connectable<annoyance, phone> // a little bit of CRTP going on here
	{
		// blah blah
	};

	class sleepless : public connectable<annoyed, sleepless>
	{
		// blah blah
	};


2) At run time, An instance of a connectable class can produce on demand an instance of its related connector type.
Such connector instances are used to facilitate the mechanics of interconnection between connectable objects.

	phone p{...};

	pc = p.get_connector<annoyance>();

	sleepless s{...};

	sc = s.get_connector<annoyed>();

	pc.mate(sc); // sc.mate(pc) would accomplish the same thing

Alternatively, connectables may be directly mated:

	pc.mate(sc);

Some things to consider:

In order to be able to participate in event exchanges, classes do not need to be derived from connectable.
They must only provide an instance of an appropriate connector type on demand. The use of the connectable 
base class template automates the construction and accessor of the connector, and enables direct mating of 
complementary connectables.

Implementing sinks, sources, and connectables

Implementing a source is trivial; one simply derives from the emitter of the associated event type:

	class beeper : public emitter<beep_event>
	{};

	beeper my_pager;
	my_pager.emit<beep_event>(10);

A sink implementation class must support an event handler of the form
void on( <event_type>, <event_type_args>...) :

	class beeped : public listener<beep_event>
	{
	public:

		void
		on(beep_event, int volume)
		{
			// do whatever it is that one does when beeped...
		}

	};

The implementation of a connectable type must derived from the appropriate connectable
base template specialization, specifying both the connector type, and the derived class
itself (the template employs CRTP techniques).

	class phone : public connectable<annoyance, phone>
	{
		...
	};

For each source type in the connector's parameter list, the connectable must include a
using-declaration to introduce the base class emit methods into the derived class to 
avoid ambiguous references:

	class phone : public connectable<annoyance, phone>
	{
	public:
		using emitter<beep_event>::emit;
		using emitter<ring_event>::emit;
		...
	};

And finally, For each sink type in the parameter list, the implementation must support
an appropriate handler:

	class sleepless : public connectable<complement<annoyance>, sleepless>
	{
	public:

		void
		on(beep_event, int volume)
		{
			...
		}

		void
		on(ring_event, int volume)
		{
			...
		}

	};

Note: though this example does not illustrate it, a single connectable class may support both sources and sinks.

Given:

	using A = event< ... >;
	using B = event< ... >;

One may:

	using fee = connector<source<A>, sink<B>>;

or, in fact, a sink and soure of the same event type:

	using fie = connector<source<A>, sink<A>>;

Note that a connector cannot be its own complement:

	using foe = complement<fie>::type;

is equivalent to:

	using foe = connector<sink<A>, source<A>>;

A source or sink for a specific event type may appear at most once
in a connector's parameter list:

	using fum = connector<sink<A>, source<B>, sink<A>> // not allowed, would result in compile errors

*/

template<class T, T Value, class... Args>
class event
{
public:
	using sink_f   = std::function<void(Args...)>;
	using source_f = std::function<void(sink_f)>;
};

template<class T>
struct is_event_type : std::false_type
{};

template<class T, T Value, class... Args>
struct is_event_type<event<T, Value, Args...>> : std::true_type
{};

template<class T, class Enable = void>
class sink;

template<class T, class Enable = void>
class source;

template<class T>
class sink<T, std::enable_if_t<is_event_type<T>::value>>
{
public:
	using event_type = T;
	friend class source<event_type>;
	using type = typename T::sink_f;

	sink() : m_sink_f{nullptr} {}

	template<class S, class = std::enable_if_t<std::is_convertible<S, type>::value>>
	sink(S&& snkf) : m_sink_f{std::forward<S>(snkf)}
	{}

	sink(sink const& rhs) : m_sink_f{rhs.m_sink_f} {}
	sink(sink&& rhs) : m_sink_f{std::move(rhs.m_sink_f)} {}

private:
	type m_sink_f;
};

template<class T>
struct is_sink : std::false_type
{};

template<class E>
struct is_sink<sink<E>> : std::true_type
{};

template<class T>
class source<T, std::enable_if_t<is_event_type<T>::value>>
{
public:
	using event_type = T;
	using type       = typename T::source_f;
	source() : m_src_f{nullptr} {}
	template<class S, class = std::enable_if_t<std::is_convertible<S, typename event_type::source_f>::value>>
	source(S&& srcf) : m_src_f{std::forward<S>(srcf)}
	{}

	source(source const& rhs) : m_src_f{rhs.m_src_f} {}
	source(source&& rhs) : m_src_f{std::move(rhs.m_src_f)} {}

	source&
	operator=(source const& rhs)
	{
		m_src_f = rhs.m_src_f;
		return *this;
	}

	source&
	operator=(source&& rhs)
	{
		m_src_f = std::move(rhs.m_src_f);
		return *this;
	}

	void
	bind(sink<event_type>&& snk)
	{
		m_src_f(std::move(snk.m_sink_f));
	}

	void
	bind(sink<event_type> const& snk)
	{
		m_src_f(snk.m_sink_f);
	}

private:
	typename event_type::source_f m_src_f;
};

template<class T>
struct is_source : std::false_type
{};

template<class E>
struct is_source<source<E>> : std::true_type
{};

namespace detail
{
template<class T, class Source>
static auto
test_get_source_method(int) -> util::traits::sfinae_true_if<decltype(std::declval<T>().template get_source<Source>())>;
template<class>
static auto
test_get_source_method(long) -> std::false_type;
}    // namespace detail

template<class T, class Source>
struct has_get_source_method : decltype(detail::test_get_source_method<T, Source>(0))
{};

namespace detail
{
template<class T, class Sink>
static auto
test_get_sink_method(int) -> util::traits::sfinae_true_if<decltype(std::declval<T>().template get_sink<Sink>())>;
template<class>
static auto
test_get_sink_method(long) -> std::false_type;
}    // namespace detail

template<class T, class Sink>
struct has_get_sink_method : decltype(detail::test_get_sink_method<T, Sink>(0))
{};

namespace _static_test
{
/* static testing of type relationships */
using test_event_0 = event<int, 0, int>;
using test_sink_0  = sink<test_event_0>;

static_assert(is_sink<test_sink_0>::value);
static_assert(is_event_type<test_event_0>::value);
static_assert(std::is_same<test_event_0, sink<test_event_0>::event_type>::value);
static_assert(std::is_same<test_event_0::sink_f, sink<test_event_0>::type>::value);
}    // namespace _static_test

template<class E>
class emitter;

template<class T, T Value, class... Args>
class emitter<event<T, Value, Args...>>
{
public:
	using event_type  = event<T, Value, Args...>;
	using source_type = source<event_type>;
	using sink_type   = sink<event_type>;

	template<class Evt>
	typename std::enable_if_t<std::is_same<Evt, event_type>::value, source<Evt>>
	get_source()
	{
		return source<Evt>{[=](auto&& snk) { this->m_sink = std::forward<decltype(snk)>(snk); }};
	}

	template<class Evt, class... Params>
	typename std::enable_if_t<std::is_same<Evt, event_type>::value>
	emit(Params&&... params)
	{
		if (m_sink)
		{
			m_sink(std::forward<Params>(params)...);
		}
	}

	template<class Evt, class Listener>
	typename std::enable_if_t<std::is_same<Evt, event_type>::value && has_get_sink_method<Listener, Evt>::value>
	bind(Listener& l)
	{
		get_source<Evt>().bind(l.template get_sink<Evt>());
	}

private:
	typename event_type::sink_f m_sink = nullptr;
};

template<class E, class Derived>
class listener;

template<class Derived, class T, T Value, class... Args>
class listener<event<T, Value, Args...>, Derived>
{
public:
	using event_type = event<T, Value, Args...>;

	template<class Evt>
	typename std::enable_if_t<std::is_same<Evt, event_type>::value, sink<Evt>>
	get_sink()
	{
		return sink<event_type>{[=](auto&&... params) mutable {
			static_cast<Derived*>(this)->on(event_type{}, std::forward<decltype(params)>(params)...);
		}};
	}

	template<class Evt, class Emitter>
	typename std::enable_if_t<std::is_same<Evt, event_type>::value && has_get_source_method<Emitter, Evt>::value>
	bind(Emitter& e)
	{
		e.template get_source<Evt>().bind(get_sink<Evt>());
	}
};

template<class T>
struct is_binding : std::integral_constant<bool, is_source<T>::value || is_sink<T>::value>
{};

template<class... Args>
struct args_are_bindings : std::false_type
{};

template<class First, class... Rest>
struct args_are_bindings<First, Rest...>
	: std::integral_constant<bool, is_binding<First>::value && args_are_bindings<Rest...>::value>
{};

template<class T>
struct args_are_bindings<T> : is_binding<T>
{};

namespace _static_test
{
static_assert(is_binding<source<test_event_0>>::value);
static_assert(args_are_bindings<source<test_event_0>, test_sink_0>::value);
}    // namespace _static_test

template<class T, class Enable = void>
struct binds_with;

template<class T>
struct binds_with<T, std::enable_if_t<is_source<T>::value>>
{
	using type = sink<typename T::event_type>;
};

template<class T>
struct binds_with<T, std::enable_if_t<is_sink<T>::value>>
{
	using type = source<typename T::event_type>;
};

// template<class T, class Emitter, class Listener>
// inline typename std::enable_if_t<has_get_source_method<Emitter, T>::value && has_get_sink_method<Listener, T>::value>
// bind(Emitter& e, Listener& l)
// {
// 	e.template get_source<T>().bind(l.template get_sink<T>());
// }

// template<class T, class Listener, class Emitter>
// inline typename std::enable_if_t<has_get_sink_method<Listener, T>::value && has_get_source_method<Emitter, T>::value>
// bind(Listener& l, Emitter& e)
// {
// 	e.template get_source<T>().bind(l.template get_sink<T>());
// }

template<class T, class Emitter, class Listener>
inline typename std::enable_if_t<has_get_source_method<Emitter, T>::value>
bind(Emitter& e, Listener& l)
{
	e.template get_source<T>().bind(l.template get_sink<T>());
}

template<class Source, class Sink>
inline typename std::enable_if_t<
		is_source<Source>::value && std::is_same<Sink, sink<typename Source::event_type>>::value>
bind(Source& src, Sink& snk)
{
	src.bind(snk);
}

template<class Sink, class Source>
inline typename std::enable_if_t<
		is_source<Source>::value && std::is_same<Sink, sink<typename Source::event_type>>::value>
bind(Sink& snk, Source& src)
{
	src.bind(snk);
}

template<std::size_t I = 0, class... Tp>
inline typename std::enable_if_t<I == sizeof...(Tp)>
bind_each(std::tuple<Tp...>& a, std::tuple<typename binds_with<Tp>::type...>& b)
{}

template<std::size_t I = 0, class... Tp>
		inline typename std::enable_if_t
		< I<sizeof...(Tp)>
		  bind_each(std::tuple<Tp...>& a, std::tuple<typename binds_with<Tp>::type...>& b)
{
	bind(std::get<I>(a), std::get<I>(b));
	bind_each<I + 1, Tp...>(a, b);
}

template<class... Args>
class connector
{
public:
	friend class connector<typename binds_with<Args>::type...>;
	static_assert(args_are_bindings<Args...>::value);
	connector(Args... args) : m_tuple{args...} {}

	void
	connect(connector<typename binds_with<Args>::type...>& that)
	{
		bind_each(m_tuple, that.m_tuple);
	}

	template<class U>
	U&
	get_binding()
	{
		return std::get<U>(m_tuple);
	}

private:
	std::tuple<Args...> m_tuple;
};

template<class T>
struct is_connector : std::false_type
{};

template<class... Args>
struct is_connector<connector<Args...>> : std::true_type
{};

template<class T>
struct complement;

template<class... Args>
struct complement<connector<Args...>>
{
	using type = connector<typename binds_with<Args>::type...>;
};

namespace _static_test
{
using test_event_1 = event<int, 1, bool>;
using test_event_2 = event<int, 2, std::string const&>;

using tcon0 = connector<source<test_event_1>, sink<test_event_2>>;

using tcon1 = complement<tcon0>::type;

static_assert(std::is_same<binds_with<source<test_event_0>>::type, sink<test_event_0>>::value);
static_assert(is_connector<tcon0>::value);
static_assert(is_connector<tcon1>::value);
static_assert(!is_connector<test_event_1>::value);
}    // namespace _static_test

template<class T, class Derived, class Enable = void>
struct bindable_base;

template<class T, class Derived>
struct bindable_base<T, Derived, std::enable_if_t<is_source<T>::value>>
{
	using type = emitter<typename T::event_type>;
};

template<class T, class Derived>
struct bindable_base<T, Derived, std::enable_if_t<is_sink<T>::value>>
{
	using type = listener<typename T::event_type, Derived>;
};

template<class T, class Derived, class Enable = void>
struct binding_initializer;

template<class T, class Derived>
struct binding_initializer<T, Derived, std::enable_if_t<is_source<T>::value>>
{
	source<typename T::event_type>
	operator()(emitter<typename T::event_type>& s) const
	{
		return s.template get_source<typename T::event_type>();
	}
};

template<class T, class Derived>
struct binding_initializer<T, Derived, std::enable_if_t<is_sink<T>::value>>
{
	sink<typename T::event_type>
	operator()(listener<typename T::event_type, Derived>& s) const
	{
		return s.template get_sink<typename T::event_type>();
	}
};

template<class Connector, class Derived>
class connectable;

template<class Derived, class... Args>
class connectable<connector<Args...>, Derived> : public bindable_base<Args, Derived>::type...
{
public:
	using connector_type = connector<Args...>;

	connectable() : m_connector{binding_initializer<Args, Derived>{}(*this)...} {}

	connectable(connectable&&)      = delete;
	connectable(connectable const&) = delete;

	template<class Connector>
	typename std::enable_if_t<std::is_same<Connector, connector_type>::value, connector_type&>
	get_connector()
	{
		return m_connector;
	}

	template<class T>
	void
	connect(T& other)
	{
		m_connector.connect(other.template get_connector<typename complement<connector_type>::type>());
	}

private:
	connector_type m_connector;
};


template<class... Args>
struct args_are_connectors : std::false_type
{};

template<class First, class... Rest>
struct args_are_connectors<First, Rest...>
	: std::integral_constant<bool, is_connector<First>::value && args_are_connectors<Rest...>::value>
{};

template<class T>
struct args_are_connectors<T> : is_connector<T>
{};

template<std::size_t I = 0, class... Tp>
inline typename std::enable_if_t<I == sizeof...(Tp)>
connect_each(std::tuple<Tp...>& a, std::tuple<typename complement<Tp>::type...>& b)
{}

template<std::size_t I = 0, class... Tp>
inline typename std::enable_if_t<(I < sizeof...(Tp))>
connect_each(std::tuple<Tp...>& a, std::tuple<typename complement<Tp>::type...>& b)
{
	std::get<I>(a).connect(std::get<I>(b));
	connect_each<I + 1, Tp...>(a, b);
}


template<class... Args>
class surface
{
public:
	friend class surface<typename complement<Args>::type...>;
	static_assert(args_are_connectors<Args...>::value);

	template<class... _Args>
	surface(_Args&&... _args) : m_tuple{std::forward<_Args>(_args)...}
	{}

	template<class T>
	T&
	get_connector()
	{
		return std::get<T>(m_tuple);
	}

	void
	stack(surface<typename complement<Args>::type...>& that)
	{
		connect_each(m_tuple, that.m_tuple);
	}

private:
	std::tuple<Args...> m_tuple;
};

template<class... Args>
struct complement<surface<Args...>>
{
	using type = surface<typename complement<Args>::type...>;
};

template<class Surface, class Derived>
class stackable;

template<class Derived, class... Args>
class stackable<surface<Args...>, Derived> : public connectable<Args, Derived>...
{
public:
	using surface_type = surface<Args...>;

	// stackable() : m_surface{connector_initializer<Args, Derived>{}(*this)...} {}

	stackable(stackable&&)      = delete;
	stackable(stackable const&) = delete;

	stackable() : m_surface{connectable<Args, Derived>::template get_connector<Args>()...} {}

	template<class T>
	T&
	get_connector()
	{
		return std::get<T>(m_surface);
	}

	template<class Surface>
	typename std::enable_if_t<std::is_same<Surface, surface_type>::value, surface_type&>
	get_surface()
	{
		return m_surface;
	}

	template<class T>
	void
	stack(T& other)
	{
		m_surface.stack(other.template get_surface<typename complement<surface_type>::type>());
	}

private:
	surface_type m_surface;
};

template<std::size_t I = 0, class... Tp>
inline typename std::enable_if_t<I == sizeof...(Tp) - 1>
assemble(std::tuple<Tp...>&)
{}

template<std::size_t I = 0, class... Tp>
inline typename std::enable_if_t<(I < (sizeof...(Tp) - 1))>
assemble(std::tuple<Tp...>& t)
{
	std::get<I>(t).get_top().stack(std::get<I + 1>(t).get_bottom());
	assemble<I + 1, Tp...>(t);
}

template<class... Args>
class assembly
{
public:
	template<class... _Args>
	assembly(_Args&&... _args) : m_tuple{std::forward<_Args>(_args)...}
	{
		assemble(m_tuple);
	}

	assembly() : m_tuple{}
	{
		assemble(m_tuple);
	}

	typename util::traits::nth_element<sizeof...(Args) - 1, Args...>&
	top()
	{
		return std::get<sizeof...(Args) - 1>(m_tuple);
	}

	typename util::traits::nth_element<0, Args...>&
	bottom()
	{
		return std::get<0>(m_tuple);
	}

	template<std::size_t N>
	typename util::traits::nth_element<N, Args...>&
	layer()
	{
		return std::get<N>(m_tuple);
	}

private:
	std::tuple<Args...> m_tuple;
};

}    // namespace event_flow
}    // namespace async

#endif    // ASYNC_EVENT_FLOW_H