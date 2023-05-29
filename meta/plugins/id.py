from cutekit.cmds import Cmd, append
from cutekit.args import Args
from uuid import uuid4


def idCmd(args: Args):
    uuid = hex(uuid4().int & (1<<64)-1)[2:]
    print(uuid)


append(Cmd(None, "id", "Generate a 64bit random id", idCmd))