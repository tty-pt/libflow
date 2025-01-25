import pyflow

class Flow:
    @staticmethod
    def write(port: int, data: bytes) -> int:
        """
        Write into a specified output port.

        Args:
            port (int): The port number.
            data (bytes): The data to write.

        Returns:
            int: The unsigned integer ID of the class.
        """
        buf = pyflow.ffi.new("char[]", data)
        result = self.lib.flow_write(port, buf, len(data))
        return result

    @staticmethod
    def read(port: int, length: int) -> int:
        """
        Read from a specified input port.

        Args:
            port (int): The port number.
            length (int): The maximum number of bytes to read.

        Returns:
            bytes: The data read from the port.
        """
        buf = self.ffi.new("char[]", length)
        read_len = self.lib.flow_read(port, buf, length)
        if read_len < 0:
            raise IOError(f"Failed to read from port {port}, error code {read_len}")
        return self.ffi.buffer(buf, read_len)[:]

    @staticmethod
    def link_tran(oinst: int, iinst: int) -> int:
        """
        Link a transition between nodes

        Args:
            oinst (int): The output node instance
            iinst (int): The input node instance

        Returns:
            int: The return code of the original API
        """
        return int(pyflow.lib.flow_link_tran(oinst, iinst))

    @staticmethod
    def link(oinst: int, oport: int, iinst: int, iport: int) -> int:
        """
        Link a transition between nodes

        Args:
            oinst (int): The output node instance
            oport (int): The output port
            iinst (int): The input node instance
            iport (int): The input port

        Returns:
            int: The return code of the original API
        """
        return int(pyflow.lib.flow_link(oinst, oport, iinst, iport))

    @staticmethod
    def node(name: str) -> int:
        """
        Load a flow or node class.

        Args:
            name (str): The name of the flow node.

        Returns:
            int: The unsigned integer ID of the class.
        """
        c_name = pyflow.ffi.new("char[]", name.encode("utf-8"))
        return int(pyflow.lib.flow_node(c_name))

    @staticmethod
    def inst(class_id: int) -> int:
        """
        Create a new instance of a node / flow.

        Args:
            class_id (int): The ID of the class to create an instance for.

        Returns:
            int: The ID of the instance.
        """
        return int(pyflow.lib.flow_inst(class_id))

    @staticmethod
    def tran(instance_id: int) -> int:
        """
        Transition into specified node instance

        Args:
            instance_id (int): The ID of the instance to transition to.

        Returns:
            int: The original API return value.
        """
        return int(pyflow.lib.flow_tran(instance_id))
