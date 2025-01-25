from flow import Flow

number_gen_0 = -1

def flow_init():
    global number_gen_0
    print("[Python] flow_init called")
    number_gen = Flow.node("number_gen");
    number_print = Flow.node("number_print");
    number_gen_0 = Flow.inst(number_gen)
    number_print_0 = Flow.inst(number_print)
    Flow.link(number_gen_0, 0, number_print_0, 0);
    Flow.link_tran(number_gen_0, number_print_0);
    return 0

def flow_run():
    global number_gen_0
    print("[Python] flow_run called")
    Flow.tran(number_gen_0)
    return 0
