/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.hh>

#include "eina_cxx_suite.h"

Eina_Error my_error, my_error_2;

EFL_START_TEST(eina_cxx_get_error)
{
  efl::eina::eina_init eina_init;

  my_error = ::eina_error_msg_static_register("Message 1");

  ::eina_error_set(0);

  efl::eina::error_code ec1 = efl::eina::get_error_code();
  ck_assert(!ec1);

  ::eina_error_set(my_error);

  efl::eina::error_code ec2 = efl::eina::get_error_code();
  ck_assert(!!ec2);

  ck_assert(ec2.message() == "Message 1");

  ::eina_error_set(ENOMEM);

  efl::eina::error_code ec3 = efl::eina::get_error_code();
  ck_assert(!!ec3);

  ck_assert(ec3.message() == strerror(ENOMEM));
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_throw_on_error)
{
  efl::eina::eina_init eina_init;

  ::eina_error_set(my_error_2);
  my_error_2 = ::eina_error_msg_static_register("Message 2");

  ::eina_error_set(0);
  try
    {
      efl::eina::throw_on_error();
    }
  catch(std::exception const&)
    {
      std::abort();
    }

  ::eina_error_set(my_error_2);
  try
    {
      efl::eina::throw_on_error();
      std::abort();
    }
  catch(efl::eina::system_error const& e)
    {
      ck_assert(e.code().value() == my_error_2);
      ck_assert(e.code().message() == "Message 2");
      ck_assert(!efl::eina::get_error_code());
    }
}
EFL_END_TEST

void
eina_test_error(TCase *tc)
{
  tcase_add_test(tc, eina_cxx_get_error);
  tcase_add_test(tc, eina_cxx_throw_on_error);
}
