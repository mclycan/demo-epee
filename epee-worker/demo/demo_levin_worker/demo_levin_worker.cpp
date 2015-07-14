// Copyright (c) 2006-2013, Andrey N. Sabelnikov, www.sabelnikov.net
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// * Neither the name of the Andrey N. Sabelnikov nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER  BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 



#include "stdafx.h"
#include "demo_levin_worker.h"
#include "console_handler.h"


template<class t_request>
bool communicate_1(net_utils::boosted_levin_async_server& transport, int id, t_request& req, const std::string& ip, const std::string& port)
{
    net_utils::connection_context_base ctx = AUTO_VAL_INIT(ctx);
    bool r = transport.connect(ip, port, 10000, ctx);
    CHECK_AND_ASSERT_MES(r, false, "failed to connect to " << ip << ":" << port);
    demo::COMMAND_WITH_ID::response rsp = AUTO_VAL_INIT(rsp);
    LOG_PRINT_RED("Request to connect to the pool. ", LOG_LEVEL_0);
    r = epee::net_utils::invoke_remote_command2(ctx.m_connection_id, id, req, rsp, transport.get_config_object());
    CHECK_AND_ASSERT_MES(r, false, "failed to invoke levin request");
    CHECK_AND_ASSERT_MES(rsp.ID_success, false, "wrong response");
    transport.get_config_object().close(ctx.m_connection_id);
    LOG_PRINT_RED("connection is ok", LOG_LEVEL_0);
  return true;
}

template<class t_request>
bool communicate_2(net_utils::boosted_levin_async_server& transport, int id, t_request& req, const std::string& ip, const std::string& port)
{
    net_utils::connection_context_base ctx = AUTO_VAL_INIT(ctx);
    bool r = transport.connect(ip, port, 10000, ctx);
    CHECK_AND_ASSERT_MES(r, false, "failed to connect to " << ip << ":" << port);
    demo::COMMAND_WITH_Result::response rsp = AUTO_VAL_INIT(rsp);
    LOG_PRINT_RED("Here is result!", LOG_LEVEL_0);
    r = epee::net_utils::invoke_remote_command2(ctx.m_connection_id, id, req, rsp, transport.get_config_object());
    CHECK_AND_ASSERT_MES(r, false, "failed to invoke levin request");
    CHECK_AND_ASSERT_MES(rsp.R_success, false, "wrong response");
    transport.get_config_object().close(ctx.m_connection_id);
    LOG_PRINT_RED("Result has been received", LOG_LEVEL_0);
  return true;
}

template<class t_request>
bool communicate_async(net_utils::boosted_levin_async_server& transport, int id, t_request& req, const std::string& ip, const std::string& port)
{
    transport.connect_async(ip, port, 10000, [&transport, id, req, ip, port](net_utils::connection_context_base& ctx, const boost::system::error_code& ec_)
    {
      if(!!ec_)
      {
        LOG_ERROR("Failed to connect to " << ip << ":" << port);
      }else
      {//connected ok!
        epee::net_utils::async_invoke_remote_command2<demo::COMMAND_WITH_Request::response>(ctx.m_connection_id, id, req, transport.get_config_object(), [&transport, ip, port](int res_code, demo::COMMAND_WITH_Request::response& rsp, net_utils::connection_context_base& ctx)
        {
          if(res_code < 0)
          {
            LOG_ERROR("Failed to invoke to " << ip << ":" << port);
          }else
          {//invoked ok
            CHECK_AND_ASSERT_MES(rsp.charset == "hello", false, "wrong response");
            LOG_PRINT_GREEN("work  is coming", LOG_LEVEL_0);
          }
          transport.get_config_object().close(ctx.m_connection_id);
          return true;
        });
        LOG_PRINT_GREEN("send work request", LOG_LEVEL_0);
      }
    });
  return true;
}

int main(int argc, char* argv[])
{
  TRY_ENTRY();
  string_tools::set_module_name_and_folder(argv[0]);

  //set up logging options
  log_space::get_set_log_detalisation_level(true, LOG_LEVEL_2);
  log_space::log_singletone::add_logger(LOGGER_CONSOLE, NULL, NULL);
  log_space::log_singletone::add_logger(LOGGER_FILE, 
                      log_space::log_singletone::get_default_log_file().c_str(), 
                      log_space::log_singletone::get_default_log_folder().c_str());



  LOG_PRINT_YELLOW("Demo server starting ...", LOG_LEVEL_0);


  demo::demo_levin_server srv;

  start_default_console(&srv, "#");

  std::string bind_param = "0.0.0.0";
  std::string port = "12345";

  if(!srv.init(port, bind_param))
  {
    LOG_ERROR("Failed to initialize srv!");
    return 1;
  }

  srv.run();
  std::string id = "mclycan";
  uint16_t num = random(6666);
  int i;
  while (!srv.is_stop())
  {
		demo::COMMAND_WITH_ID::request req1;
		req1.ID_data = id;
		req1.ID_num = num;
		bool r = communicate_1(srv.get_server(), demo::COMMAND_WITH_ID::ID, req1, "127.0.0.1", port);
		misc_utils::sleep_no_w(3201);

	
		demo::COMMAND_WITH_Request::request req2;
		req2.ID_data = id;
		req2.ID_num = num;
		r = communicate_async(srv.get_server(), demo::COMMAND_WITH_Request::ID, req2, "127.0.0.1", port);

		i = 0;
		misc_utils::sleep_no_w(15201);
		i = 1;
		//do some work here;

		if(i);
		{
			demo::COMMAND_WITH_Result::request req3;
			req3.ID_data = id;
			req3.ID_num = num;
			req3.ID_task = 1010;
			req3.passwd = "hello world!";
			r = communicate_2(srv.get_server(), demo::COMMAND_WITH_Result::ID, req3, "127.0.0.1", port);
			misc_utils::sleep_no_w(3201);
		}

  }
  bool r = srv.wait_stop();
  CHECK_AND_ASSERT_MES(r, 1, "failed to wait server stop");


  srv.deinit();

  LOG_PRINT_YELLOW("Demo server stoped.", LOG_LEVEL_0);
  return 1;

  CATCH_ENTRY_L0("main", 1);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace demo
{
  bool demo_levin_server::init(const std::string& bind_port, const std::string& bind_ip)
  {
    m_net_server.get_config_object().m_pcommands_handler = this;
    LOG_PRINT_L0("Binding on " << bind_ip << ":" << bind_port);
    return m_net_server.init_server(bind_port, bind_ip);
  }

  bool demo_levin_server::run()
  {
    m_stop = false;
    //here you can set worker threads count
    int thrds_count = 1;
    m_net_server.get_config_object().m_invoke_timeout = 10000;
    m_net_server.get_config_object().m_pcommands_handler = this;

    //go to loop
    LOG_PRINT_YELLOW("Run net_service loop( " << thrds_count << " threads)...", LOG_LEVEL_0);
    if(!m_net_server.run_server(thrds_count, false))
    {
      LOG_ERROR("Failed to run net tcp server!");
    }

    LOG_PRINT_YELLOW("net_service loop stopped.", LOG_LEVEL_0);
    return true;
  }

  bool demo_levin_server::deinit()
  {
    return m_net_server.deinit_server();
  }

  bool demo_levin_server::send_stop_signal()
  {
    m_net_server.send_stop_signal();
    return true;
  }

  int demo_levin_server::handle_command_1(int command, COMMAND_EXAMPLE_1::request& arg, COMMAND_EXAMPLE_1::response& rsp, const net_utils::connection_context_base& context)
  {
    
    return 1;
  }

  int demo_levin_server::handle_command_2(int command, COMMAND_EXAMPLE_2::request& arg, COMMAND_EXAMPLE_2::response& rsp, const net_utils::connection_context_base& context)
  {
    return 1;
  }
  int demo_levin_server::handle_notify_1(int command, COMMAND_EXAMPLE_1::request& arg, const net_utils::connection_context_base& context)
  {
    return 1;
  }
  int demo_levin_server::handle_notify_2(int command, COMMAND_EXAMPLE_2::request& arg, const net_utils::connection_context_base& context)
  {
    return 1;
  }
}
