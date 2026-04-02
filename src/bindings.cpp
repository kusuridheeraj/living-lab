#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "core.cpp"

namespace py = pybind11;

PYBIND11_MODULE(living_limiter_core, m) {
    m.doc() = "Living Lab - living-limiter core high-performance engine";

    py::class_<living_limiter::TokenBucket>(m, "TokenBucket")
        .def(py::init<long long, double>())
        .def("check", &living_limiter::TokenBucket::check, py::arg("requested") = 1)
        .def("get_tokens", &living_limiter::TokenBucket::get_tokens);

    py::class_<living_limiter::LeasedTokenBucket>(m, "LeasedTokenBucket")
        .def(py::init<long long, double>(), py::arg("batch_size"), py::arg("jitter_factor") = 0.1)
        .def("check", &living_limiter::LeasedTokenBucket::check, py::arg("requested") = 1)
        .def("top_up", &living_limiter::LeasedTokenBucket::top_up, py::arg("new_tokens"))
        .def("is_renewal_needed", &living_limiter::LeasedTokenBucket::is_renewal_needed)
        .def("get_tokens", &living_limiter::LeasedTokenBucket::get_tokens);

    py::class_<living_limiter::SlidingWindowLog>(m, "SlidingWindowLog")
        .def(py::init<long long, long long>())
        .def("check", &living_limiter::SlidingWindowLog::check);

    py::class_<living_limiter::LeakyBucket>(m, "LeakyBucket")
        .def(py::init<long long, double>())
        .def("check", &living_limiter::LeakyBucket::check)
        .def("get_level", &living_limiter::LeakyBucket::get_level);

    py::class_<living_limiter::FixedWindow>(m, "FixedWindow")
        .def(py::init<long long, long long>())
        .def("check", &living_limiter::FixedWindow::check);

    py::class_<living_limiter::ProbabilisticShield>(m, "ProbabilisticShield")
        .def(py::init<size_t, int>())
        .def("check", &living_limiter::ProbabilisticShield::check)
        .def("reset", &living_limiter::ProbabilisticShield::reset);
}
