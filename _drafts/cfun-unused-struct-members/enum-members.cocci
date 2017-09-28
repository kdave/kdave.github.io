/*
 * Capture and print all struct members.
 *
 * Output:
 * STRUCT: name member
 */

@m@
identifier STRUCT, MEMBER;
type MTYPE;
@@

* struct STRUCT {
  ...
* MTYPE MEMB;
  ...
  }

@script:python@
stru << m.STRUCT;
memb << m.MEMBER;
@@
print "STRUCT: %s %s" % (stru, memb)
