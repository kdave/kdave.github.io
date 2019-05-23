struct empty {
};

struct struct_int {
	int a;
};

struct struct_int_multi {
	int a;
	int b;
	int c;
};

struct struct_with_anon_struct {
	int a;
	struct {
		int aa;
		int bb;
	};
	int b;
};

struct struct_with_anon_union {
	int a;
	union {
		int aa;
		int bb;
	};
	int b;
};

struct struct_with_named_struct {
	int a;
	struct struct_inside {
		int aa;
		int bb;
	};
	int b;
};

struct struct_with_named_union {
	int a;
	union union_inside {
		int aa;
		int bb;
	};
	int b;
};
