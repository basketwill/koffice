class MyClass
{
	MyClass( in this )
	{
	}

	clicked( in this )
	{
		println( "Yeah, click me, faster, harder ....." );
	}
};

main
{
	r = QRect();
	r.left = 10;
	r.right = 180;
	r.top = 50;
	r.bottom = 110;
	a = QApplication();
	w5 = QFrame();
	w5.geometry = r;
	w5.frameshape = "Panel";
	w5.frameshadow = "Sunken";
	w5.show();
	println( w5.isA() );
	println( w5.inherits( "qt:QObject" ) );
	println( w5.inherits( "qt:QWidget" ) );
	println( w5.inherits( "qt:QFrame" ) );
	println( w5.inherits( "qt:QApplication" ) );
	println( w5.inherits( "MyClass" ) );
	println( "-----------------------" );
	m = MyClass();
	println( m.isA() );
	println( m.inherits( "MyClass" ) );

	b = QPushButton();
	b.text = "Click me";
	connect( b.clicked, m.clicked );	
	b.show();

	grp = QGroupBox( 1, "Vertical", "Meine Box" );
	x = QPushButton( grp );
	x.text = "Button 1";
	y = QPushButton( grp );
	y.text = "Button 2";
	grp.show();

	a.exec();
}
