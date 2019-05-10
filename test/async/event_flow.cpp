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

#include <doctest.h>
#include <iostream>
#include <async/event_flow.h>

using namespace async;
using namespace event_flow;

#if 1
namespace event_flow_test
{
enum class actions
{
	start,
	stop,
	kill
};

using kill_event  = event<actions, actions::kill, std::string const&>;
using start_event = event<actions, actions::start, int>;
using stop_event  = event<actions, actions::stop, bool>;

class killer : public emitter<kill_event>
{};

class victim : public listener<kill_event, victim>
{
public:
	void
	on(kill_event, std::string const& msg)
	{
		m_is_dead = true;
		m_message = msg;
	}

	bool
	is_dead() const
	{
		return m_is_dead;
	}

	std::string const&
	message() const
	{
		return m_message;
	}

private:
	bool        m_is_dead;
	std::string m_message;
};

class functor_victim
{
public:
	functor_victim() : m_is_dead{false}, m_message{} {}

	void
	die(std::string const& message)
	{
		m_is_dead = true;
		m_message = message;
	}

	bool
	is_dead() const
	{
		return m_is_dead;
	}

	std::string const&
	message() const
	{
		return m_message;
	}

	struct kill_functor
	{
		kill_functor(functor_victim& ksrc) : m_victim{ksrc} {}

		functor_victim& m_victim;

		void
		operator()(std::string const& s)
		{
			m_victim.die(s);
		}
	};

	template<class T>
	std::enable_if_t<std::is_same<T, kill_event>::value, sink<T>>
	get_sink()
	{
		return sink<kill_event>{kill_functor(*this)};
	}

	template<class T, class Emitter>
	std::enable_if_t<std::is_same<T, kill_event>::value && has_get_source_method<Emitter, T>::value>
	bind(Emitter& e)
	{
		e.template get_source<T>().bind(get_sink<T>());
	}

	bool        m_is_dead;
	std::string m_message;
};

class lambda_victim
{
public:
	lambda_victim() : m_is_dead{false}, m_message{} {}

	void
	die(std::string const& message)
	{
		m_is_dead = true;
		m_message = message;
	}

	bool
	is_dead() const
	{
		return m_is_dead;
	}

	std::string const&
	message() const
	{
		return m_message;
	}

	template<class T>
	std::enable_if_t<std::is_same<T, kill_event>::value, sink<T>>
	get_sink()
	{
		return sink<kill_event>{[=](std::string const& s) { this->die(s); }};
	}

	template<class T, class Emitter>
	std::enable_if_t<std::is_same<T, kill_event>::value && has_get_source_method<Emitter, T>::value>
	bind(Emitter& e)
	{
		e.template get_source<T>().bind(get_sink<T>());
	}

	bool        m_is_dead;
	std::string m_message;
};

using snuff = connector<source<kill_event>, sink<kill_event>>;

class murder : public connectable<snuff, murder>
{
public:
	void
	on(kill_event, std::string const& msg)
	{
		m_is_dead = true;
		m_message = msg;
	}

	bool
	is_dead() const
	{
		return m_is_dead;
	}

	std::string const&
	message() const
	{
		return m_message;
	}

private:
	bool        m_is_dead = false;
	std::string m_message;
};

using ffuns = complement<snuff>::type;

class redrum : public connectable<complement<snuff>::type, redrum>
{
public:
	void
	on(kill_event, std::string const& msg)
	{
		m_is_dead = true;
		m_message = msg;
	}

	bool
	is_dead() const
	{
		return m_is_dead;
	}

	std::string const&
	message() const
	{
		return m_message;
	}

private:
	bool        m_is_dead = false;
	std::string m_message;
};

enum class noise
{
	beep,
	ring,
	squeal,
	honk
};

using beep_event   = event<noise, noise::beep, bool, int>;
using ring_event   = event<noise, noise::ring, bool, int>;
using squeal_event = event<noise, noise::squeal, std::string const&, char>;
using honk_event   = event<noise, noise::honk, std::shared_ptr<int>>;

using foo_con = connector<source<beep_event>, source<ring_event>, sink<squeal_event>, sink<honk_event>>;

class foo : public connectable<foo_con, foo>
{
public:
	using emitter<beep_event>::emit;
	using emitter<ring_event>::emit;

	void
	on(squeal_event, std::string const& s, char c)
	{
		squeal_string = s;
		squeal_char   = c;
	}

	void
	on(honk_event, std::shared_ptr<int> p)
	{
		honk_ptr = p;
	}

	std::string squeal_string;
	char        squeal_char = ' ';

	std::shared_ptr<int> honk_ptr = nullptr;
};

using oof_con = complement<foo_con>::type;

class oof : public connectable<complement<foo_con>::type, oof>
{
public:
	using emitter<squeal_event>::emit;
	using emitter<honk_event>::emit;

	void
	on(beep_event, bool b, int i)
	{
		beep_flag = b;
		beep_num  = i;
	}

	void
	on(ring_event, bool b, int i)
	{
		ring_flag = b;
		ring_num  = i;
	}

	bool beep_flag = false;
	int  beep_num  = 0;
	bool ring_flag = false;
	int  ring_num  = 0;
};

class combo_b;

class combo_a : public connectable<foo_con, combo_a>, public connectable<oof_con, combo_a>
{
public:
	using foo_base = connectable<foo_con, combo_a>;
	using oof_base = connectable<oof_con, combo_a>;

	using emitter<squeal_event>::emit;
	using emitter<honk_event>::emit;

	using emitter<beep_event>::emit;
	using emitter<ring_event>::emit;

	using foo_base::get_connector;
	using oof_base::get_connector;
	using foo_base::connect;
	using oof_base::connect;

	combo_a(combo_b& b);


	bool                 beep_flag;
	int                  beep_i;
	bool                 ring_flag;
	int                  ring_i;
	std::string          str;
	char                 ch;
	std::shared_ptr<int> intp;

	void
	send_beep()
	{
		emit<beep_event>(false, 1);
	}

	void
	send_ring()
	{
		emit<ring_event>(true, 2);
	}

	void
	on(beep_event, bool b, int i)
	{
		beep_flag = b;
		beep_i    = i;
	}

	void
	on(ring_event, bool b, int i)
	{
		ring_flag = b;
		ring_i    = i;
	}

	void
	send_squeal()
	{
		emit<squeal_event>("hello", 'Z');
	}

	void
	on(squeal_event, std::string const& s, char c)
	{
		str = s;
		ch  = c;
	}

	void
	send_honk()
	{
		emit<honk_event>(std::make_shared<int>(3));
	}

	void
	on(honk_event, std::shared_ptr<int> ip)
	{
		intp = ip;
	}
};

class combo_b : public connectable<foo_con, combo_b>, public connectable<oof_con, combo_b>
{
public:
	using foo_base = connectable<foo_con, combo_b>;
	using oof_base = connectable<oof_con, combo_b>;

	using emitter<squeal_event>::emit;
	using emitter<honk_event>::emit;

	using emitter<beep_event>::emit;
	using emitter<ring_event>::emit;

	using foo_base::get_connector;
	using oof_base::get_connector;
	using foo_base::connect;
	using oof_base::connect;

	void
	on(beep_event, bool b, int i)
	{
		emit<beep_event>(b, i);
	}

	void
	on(ring_event, bool b, int i)
	{
		emit<ring_event>(b, i);
	}

	void
	on(squeal_event, std::string const& s, char c)
	{
		emit<squeal_event>(s, c);
	}

	void
	on(honk_event, std::shared_ptr<int> ip)
	{
		emit<honk_event>(ip);
	}
};

}    // namespace event_flow_test

event_flow_test::combo_a::combo_a(combo_b& b)
{
	get_connector<foo_con>().connect(b.get_connector<oof_con>());
	get_connector<oof_con>().connect(b.get_connector<foo_con>());
}


using namespace event_flow_test;

TEST_CASE("async::event_flow [ smoke ] { functor }")
{

	functor_victim nicole;

	killer oh_jay;

	// oh_jay.get_source<kill_event>().bind(nicole.get_sink<kill_event>());
	// bind<kill_event>(oh_jay, nicole);
	// oh_jay.bind<kill_event>(nicole);
	nicole.bind<kill_event>(oh_jay);

	oh_jay.emit<kill_event>("Die, bitch!");


	CHECK(nicole.is_dead());
	CHECK(nicole.message() == "Die, bitch!");
}

TEST_CASE("async::event_flow [ smoke ] { naked emitter }")
{

	functor_victim nicole;

	emitter<kill_event> oh_jay;

	// oh_jay.get_source<kill_event>().bind(nicole.get_sink<kill_event>());
	// bind<kill_event>(oh_jay, nicole);
	// oh_jay.bind<kill_event>(nicole);
	oh_jay.bind<kill_event>(nicole);

	oh_jay.emit<kill_event>("Die, bitch!");

	CHECK(nicole.is_dead());
	CHECK(nicole.message() == "Die, bitch!");
}

TEST_CASE("async::event_flow::sink [ smoke ] { lambda move }")
{
	lambda_victim nicole;

	killer oh_jay;

	// oh_jay.get_source<kill_event>().bind(nicole.get_sink<kill_event>());
	nicole.bind<kill_event>(oh_jay);

	oh_jay.emit<kill_event>("Die, bitch!");


	CHECK(nicole.is_dead());
	CHECK(nicole.message() == "Die, bitch!");
}

TEST_CASE("async::event_flow::connector [ smoke ] { 1 }")
{
	lambda_victim  nicole;
	functor_victim jfk;

	killer oh_jay;
	killer lee_harvey;

	connector<source<kill_event>, sink<kill_event>> left{oh_jay.get_source<kill_event>(), jfk.get_sink<kill_event>()};
	connector<sink<kill_event>, source<kill_event>> right{nicole.get_sink<kill_event>(),
														  lee_harvey.get_source<kill_event>()};
	left.connect(right);

	oh_jay.emit<kill_event>("Die, bitch!");
	lee_harvey.emit<kill_event>("Bang, bang bang!");


	CHECK(nicole.is_dead());
	CHECK(nicole.message() == "Die, bitch!");

	CHECK(jfk.is_dead());
	CHECK(jfk.message() == "Bang, bang bang!");
}

TEST_CASE("async::event_flow::connector [ smoke ] { 2 }")
{
	victim nicole;

	killer oh_jay;

	oh_jay.get_source<kill_event>().bind(nicole.get_sink<kill_event>());

	oh_jay.emit<kill_event>("Die, bitch!");


	CHECK(nicole.is_dead());
	CHECK(nicole.message() == "Die, bitch!");
}


TEST_CASE("async::event_flow::connectable [ smoke ] { simple }")
{
	murder top;
	redrum bottom;

	top.connect(bottom);

	//	top.get_connector< snuff >().connect( bottom.get_connector< ffuns >() );

	top.emit<kill_event>("Die, bitch!");
	bottom.emit<kill_event>("Bang, bang bang!");


	CHECK(top.is_dead());
	CHECK(top.message() == "Bang, bang bang!");

	CHECK(bottom.is_dead());
	CHECK(bottom.message() == "Die, bitch!");
}

TEST_CASE("async::event_flow::connectable [ smoke ] { complex }")
{
	foo f;
	oof o;

	f.get_connector<foo_con>().connect(o.get_connector<oof_con>());

	f.emit<beep_event>(true, 27);
	f.emit<ring_event>(true, 108);


	CHECK(o.beep_flag == true);
	CHECK(o.beep_num == 27);
	CHECK(o.ring_flag == true);
	CHECK(o.ring_num == 108);

	o.emit<squeal_event>("zoot", 'x');
	o.emit<honk_event>(std::make_shared<int>(42));

	CHECK(f.squeal_string == "zoot");
	CHECK(f.squeal_char == 'x');
	CHECK(*f.honk_ptr == 42);
}

namespace event_flow_test
{
using top_surface = surface<foo_con, oof_con>;

using bottom_surface = surface<oof_con, foo_con>;

class top : public stackable<top_surface, top>
{
public:
	using foo_base = connectable<foo_con, top>;
	using oof_base = connectable<oof_con, top>;

	using emitter<squeal_event>::emit;
	using emitter<honk_event>::emit;

	using emitter<beep_event>::emit;
	using emitter<ring_event>::emit;

	// using foo_base::get_connector;
	// using oof_base::get_connector;
	// using foo_base::connect;
	// using oof_base::connect;

	bool                 beep_flag;
	int                  beep_i;
	bool                 ring_flag;
	int                  ring_i;
	std::string          str;
	char                 ch;
	std::shared_ptr<int> intp;

	void
	send_beep()
	{
		emit<beep_event>(false, 1);
	}

	void
	send_ring()
	{
		emit<ring_event>(true, 2);
	}

	void
	on(beep_event, bool b, int i)
	{
		beep_flag = b;
		beep_i    = i;
	}

	void
	on(ring_event, bool b, int i)
	{
		ring_flag = b;
		ring_i    = i;
	}

	void
	send_squeal()
	{
		emit<squeal_event>("hello", 'Z');
	}

	void
	on(squeal_event, std::string const& s, char c)
	{
		str = s;
		ch  = c;
	}

	void
	send_honk()
	{
		emit<honk_event>(std::make_shared<int>(3));
	}

	void
	on(honk_event, std::shared_ptr<int> ip)
	{
		intp = ip;
	}
};

class bottom : public stackable<bottom_surface, bottom>
{
public:
	using foo_base = connectable<foo_con, top>;
	using oof_base = connectable<oof_con, top>;

	using emitter<squeal_event>::emit;
	using emitter<honk_event>::emit;

	using emitter<beep_event>::emit;
	using emitter<ring_event>::emit;

	// using foo_base::get_connector;
	// using oof_base::get_connector;
	// using foo_base::connect;
	// using oof_base::connect;

	void
	on(beep_event, bool b, int i)
	{
		emit<beep_event>(b, i);
	}

	void
	on(ring_event, bool b, int i)
	{
		emit<ring_event>(b, i);
	}

	void
	on(squeal_event, std::string const& s, char c)
	{
		emit<squeal_event>(s, c);
	}

	void
	on(honk_event, std::shared_ptr<int> ip)
	{
		emit<honk_event>(ip);
	}
};

}    // namespace event_flow_test

TEST_CASE("async::event_flow::connectable [ smoke ] { double wrap }")
{
	combo_b b;
	combo_a a(b);

	a.send_beep();
	a.send_ring();
	a.send_squeal();
	a.send_honk();


	CHECK(a.beep_flag == false);
	CHECK(a.beep_i == 1);
	CHECK(a.ring_flag == true);
	CHECK(a.ring_i == 2);
	CHECK(a.str == "hello");
	CHECK(a.ch == 'Z');
	CHECK(*a.intp == 3);

	b.emit<beep_event>(true, 27);
	CHECK(a.beep_flag == true);
	CHECK(a.beep_i == 27);
}

TEST_CASE("async::event_flow::surface [ smoke ] { basic }")
{
	top    t;
	bottom b;

	// t.get_surface<top_surface>().stack(b.get_surface<bottom_surface>());

	t.stack(b);

	t.send_beep();
	t.send_ring();
	t.send_squeal();
	t.send_honk();


	CHECK(t.beep_flag == false);
	CHECK(t.beep_i == 1);
	CHECK(t.ring_flag == true);
	CHECK(t.ring_i == 2);
	CHECK(t.str == "hello");
	CHECK(t.ch == 'Z');
	CHECK(*t.intp == 3);
}

namespace stack_test
{
using string_event = event<std::size_t, 0, std::string const&>;

using str_in     = connector<sink<string_event>>;
using str_out    = connector<source<string_event>>;
using str_top    = surface<str_in, str_out>;
using str_bottom = complement<str_top>::type;

class repeater
{
private:
	class bottom;

	class top : public stackable<str_top, top>
	{
	public:
		top(bottom* b) : m_bottom{b} {}
		void
		on(string_event, std::string const& s);

	private:
		bottom* m_bottom;
	};

	class bottom : public stackable<str_bottom, bottom>
	{
	public:
		bottom(top* t) : m_top{t} {}
		void
		on(string_event, std::string const& s)
		{
			m_top->emit<string_event>(s);
		}

	private:
		top* m_top;
	};

	top    m_top;
	bottom m_bottom;

public:
	repeater() : m_top{&m_bottom}, m_bottom{&m_top} {}
	repeater(repeater&& rhs) : m_top{&m_bottom}, m_bottom{&m_top} {}
	repeater(repeater const& rhs) : m_top{&m_bottom}, m_bottom{&m_top} {}

	str_top&
	get_top()
	{
		return m_top.get_surface<str_top>();
	}

	str_bottom&
	get_bottom()
	{
		return m_bottom.get_surface<str_bottom>();
	}
};

void
repeater::top::on(string_event, std::string const& s)
{
	m_bottom->emit<string_event>(s);
}

class anchor : public stackable<str_top, anchor>
{
public:
	anchor() : stackable<str_top, anchor>{} {}
	anchor(anchor&& rhs) : stackable<str_top, anchor>{} {}
	anchor(anchor const& rhs) : stackable<str_top, anchor>{} {}

	void
	on(string_event, std::string const& s)
	{
		std::cout << "anchor received " << s << std::endl;
		emit<string_event>(s);
	}

	str_top&
	get_top()
	{
		return get_surface<str_top>();
	}
};

class driver : public stackable<str_bottom, driver>
{
public:
	driver() : stackable<str_bottom, driver>{} {}
	driver(driver&& rhs) : stackable<str_bottom, driver>{}, m_handler{std::move(rhs.m_handler)} {}
	driver(driver const& rhs) : stackable<str_bottom, driver>{}, m_handler{rhs.m_handler} {}

	using handler = std::function<void(std::string const&)>;

	void
	on_string(handler h)
	{
		m_handler = h;
	}

	str_bottom&
	get_bottom()
	{
		return get_surface<str_bottom>();
	}

	void
	on(string_event, std::string const& s)
	{
		m_handler(s);
	}

private:
	handler m_handler;
};

}    // namespace stack_test

TEST_CASE("async::event_flow::surface [ smoke ] { layer stacking null constructed tuple }")
{
	assembly<stack_test::anchor, stack_test::repeater, stack_test::driver> stck;

	bool handler_called{false};

	stck.top().on_string([&](std::string const& s) {
		std::cout << "got " << s << " from top of stack" << std::endl;
		CHECK(s == "zoot");
		handler_called = true;
	});

	stck.top().emit<stack_test::string_event>("zoot");
	CHECK(handler_called);
}


TEST_CASE("async::event_flow::surface [ smoke ] { layer stacking move constructed tuple }")
{
	assembly<stack_test::anchor, stack_test::repeater, stack_test::driver> stck{
			stack_test::anchor{}, stack_test::repeater{}, stack_test::driver{}};

	bool handler_called{false};

	stck.top().on_string([&](std::string const& s) {
		std::cout << "got " << s << " from top of stack" << std::endl;
		CHECK(s == "zoot");
		handler_called = true;
	});

	stck.top().emit<stack_test::string_event>("zoot");
	CHECK(handler_called);
}

TEST_CASE("async::event_flow::surface [ smoke ] { layer stacking manual }")
{
	std::tuple<stack_test::anchor, stack_test::repeater, stack_test::driver> stck;
	std::get<0>(stck).get_top().stack(std::get<1>(stck).get_bottom());
	std::get<1>(stck).get_top().stack(std::get<2>(stck).get_bottom());

	std::get<2>(stck).on_string([=](std::string const& s) {
		std::cout << "got " << s << " from top of stack" << std::endl;
		CHECK(s == "zoot");
	});

	std::get<2>(stck).emit<stack_test::string_event>("zoot");
}

#endif
